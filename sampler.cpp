/*
Copyright (c) 2009, Madis Kaal <mast@nomad.ee> 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the involved organizations nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
#include "ligrid.hpp"

void CSampler::SetOptions(int invertleft,int swapchannels)
{
  this->invertleft=invertleft;
  this->swapchannels=swapchannels;
}  

// misleading name, this gives number of samples taken in 24 hours
//
unsigned long CSampler::GetClockDrift()
{
  if (realrate)
    return realrate; // if precalculated, use it
  return (unsigned long)((24.0*60.0*60.0*(samplerate>32000?
      (double)(samplerate/2):(double)samplerate))/22.05);
}

// estimates the current value of sample clock from last clock
// value, and time elapsed since it was last changed
//
long CSampler::GetClock()
{
  mutex.Lock();
  struct timeval ve,vs;
  long c=clock;
  vs=tv;
  gettimeofday(&ve,NULL);
  mutex.Unlock();
  if (ve.tv_usec<vs.tv_usec) {
    ve.tv_usec+=1000000;
    ve.tv_sec-=1;
  }
  unsigned long d=(ve.tv_sec-vs.tv_sec)*1000000+(ve.tv_usec-vs.tv_usec);
  return c+(d/45);
}

CSampler::CSampler()
{
  // filter coefficents for high-pass filter that suppresses
  // mains noise
  // TODO: these were experimentally found, should the filter be better?
  double fc[]= {
    0.100,
    -0.400,
    0.600,
    -0.400,
    0.100,
  };
  // filter coefficents for low-pass filter that is used for
  // anti-aliasing before downsampling. should cut sharply 
  // above 40% of sample rate
  //
  double lpfc[] = {
    -0.00000000007507,
    -0.00000763429300,
    -0.00004041223300,
    0.00001971907300,
    0.00020377662000,
    0.00013767974000,
    -0.00040439283000,
    -0.00066447260000,
    0.00029251078000,
    0.00153570750000,
    0.00067007710000,
    -0.00221148090000,
    -0.00283535780000,
    0.00155079780000,
    0.00575083080000,
    0.00176655350000,
    -0.00765895660000,
    -0.00823938200000,
    0.00564104100000,
    0.01634167500000,
    0.00329197220000,
    -0.02180879600000,
    -0.02045659200000,
    0.01784535700000,
    0.04400624000000,
    0.00468221960000,
    -0.06873457000000,
    -0.06387941500000,
    0.08764380000000,
    0.30289385000000,
    0.40517990000000,
    0.30289385000000,
    0.08764380000000,
    -0.06387941500000,
    -0.06873457000000,
    0.00468221960000,
    0.04400624000000,
    0.01784535700000,
    -0.02045659200000,
    -0.02180879600000,
    0.00329197220000,
    0.01634167500000,
    0.00564104100000,
    -0.00823938200000,
    -0.00765895660000,
    0.00176655350000,
    0.00575083080000,
    0.00155079780000,
    -0.00283535780000,
    -0.00221148090000,
    0.00067007710000,
    0.00153570750000,
    0.00029251078000,
    -0.00066447260000,
    -0.00040439283000,
    0.00013767974000,
    0.00020377662000,
    0.00001971907300,
    -0.00004041223300,
    -0.00000763429300,
    -0.00000000007507
  };
  left=new FIR(COUNTOF(fc),fc);
  right=new FIR(COUNTOF(fc),fc);
  leftlp=new FIR(COUNTOF(lpfc),lpfc);
  rightlp=new FIR(COUNTOF(lpfc),lpfc);
  Init();
}

void CSampler::Init()
{
  ifd=-1;
  memset(block,0,sizeof(block));
  memset(scanblock,0,sizeof(scanblock));
  triggerlevel=100;
  clock=0;
  gettimeofday(&tv,NULL);
  autotriggerlevel=0.0;
  invertleft=swapchannels=0;
  blocksread=0;
  realrate=0;
}

CSampler::~CSampler()
{
  delete left;
  delete right;
  Close();
}

bool CSampler::Open(int volume,const char *device,int samplerate,
  CStereoInput::RECORDINGSOURCE input)
{
  Close();
  Init();
  this->samplerate=samplerate;
  if (StereoInputDevice.Open(device)) {
    if (StereoInputDevice.SetRecordingSource(input)) {
      if (StereoInputDevice.SetRecordingVolume(volume)) {
        if (StereoInputDevice.SetSampleRate(samplerate)) {
          return true;
        }
      }
    }
  }
  return false;
}

void CSampler::Close() 
{
  StereoInputDevice.Close();
}

// read next block of samples, downsample 2x if sampling rate is
// above 32kHz, and apply simple preprocessing (filtering and wiring
// adjustments)
//
int CSampler::ReadBlock()
{
  uint i,j=samplerate>32000?BLOCK_SIZE*2:BLOCK_SIZE;
  timeval ttv;
  short l,r,x,*pcm,*pp;
  i=StereoInputDevice.Read(block,j);
  if (i!=j)
    Log.Write("Audio data read read error (%s)",strerror(errno));
  else {
    if (!blocksread)
      gettimeofday(&first_tv,NULL);
    blocksread++;
    if (!realrate && blocksread==10000) {
      gettimeofday(&ttv,NULL);
      if (ttv.tv_usec<first_tv.tv_usec) {
        ttv.tv_usec+=1000000;
        ttv.tv_sec-=1;
      }
      // calculate d as diff of times microseconds
      unsigned long d=(ttv.tv_sec-first_tv.tv_sec)*1000000+
        (ttv.tv_usec-first_tv.tv_usec);
      unsigned long s=(blocksread-1)*(BLOCK_SIZE/4); // number of samples read
      double sps=(double)s*1000000.0/(double)d;
      realrate=(int)((sps*24.0*60.0*60.0)/22.05);
      Log.Write("Sample interval is %f usec",(double)d/(double)s);
      Log.Write("%f samples per second",sps);
      Log.Write("%u samples per day",realrate);
    }
    // if we are sampling at 44.1 or 48 khz then apply
    // low-pass filter to satisfy nyquist, and downsample
    if (samplerate>32000) {
      pcm=(short*)block;
      for (i=0;i<sizeof(block);i+=4) {
        l=pcm[0];
        r=pcm[1];
        *pcm=(short)(leftlp->Filter(l)+0.5);
        pcm++;
        *pcm=(short)(rightlp->Filter(r)+0.5);
        pcm++;
      }
      pcm=pp=(short*)block;
      for (i=0;i<(sizeof(block)/2);i+=4) {
        pcm[0]=(pp[0]+pp[2])/2;
        pcm[1]=(pp[1]+pp[3])/2;
        pcm+=2;
        pp+=4;
      }
    }
    // now process the real signal
    pcm=(short*)block;
    for (i=0;i<BLOCK_SIZE;i+=4) {
      l=pcm[0];
      r=pcm[1];
      if (invertleft) // adjust for wiring problems
        l=-l;
      if (swapchannels) {
        x=l;
        l=r;
        r=x;
      }
      *pcm=(short)(left->Filter(l));
      pcm++;
      *pcm=(short)(right->Filter(r));
      pcm++;
    }
    AddData(block,BLOCK_SIZE,clock);
    mutex.Lock();
    clock+=BLOCK_SIZE/4; // start of next block clock value
    gettimeofday(&tv,NULL);
    mutex.Unlock();
  }
  return i;
}

bool CSampler::AddData(uchar *data,uint nbytes,long clk)
{
  void *s;
  s=new uchar[BLOCK_SIZE+4];
  if (s) {
    *(long*)s=clk;
    memcpy((uchar *)s+4,data,BLOCK_SIZE);
    return blockqueue.Add(s);
  }
  return false;
}

void CSampler::SetAutoTriggerLevel(double multiplier)
{
  autotriggerlevel=multiplier;
  Log.Write("Auto trigger multiplier set to %f",autotriggerlevel);
}

void CSampler::SetTriggerLeveldB(double leveldB)
{
  if (leveldB>0)
    leveldB=0-leveldB;
  triggerlevel=pow(10,(leveldB/20.0))*32768.0;
  Log.Write("Trigger level set to %f",triggerlevel);
}

void CSampler::SetTriggerLevel(double level)
{
  triggerlevel=level;
  Log.Write("Trigger level set to %f",triggerlevel);
}

double CSampler::GetTriggerLevel()
{
  if (!autotriggerlevel)
    return triggerlevel;
  return avgpower.Get()*autotriggerlevel;
}

double CSampler::GetTriggerLeveldB()
{
  return 20.0*log10(GetTriggerLevel()/32768.0);
}

// data processing takes place in two sequential data blocks
// to allow taking pre-strike samples for strikes happening
// very early in data block
//
void CSampler::ProcessData()
{
  uchar *s;
  long clk;
  int i,left,right;
  double power;
  while (blockqueue.Count()) {
    s=(uchar *)blockqueue.Get();
    if (s) {
      // finish up partial strikes first        
      strikes.FinishStrikes(s+4,BLOCK_SIZE);
      memcpy(scanblock+BLOCK_SIZE,s+4,BLOCK_SIZE);
      clk=*(long*)s;
      delete s;
      for (i=0;i<BLOCK_SIZE;i+=4) {
        left=(short)(scanblock[i+BLOCK_SIZE]|(scanblock[i+BLOCK_SIZE+1]<<8));
        right=(short)(scanblock[i+BLOCK_SIZE+2]|(scanblock[i+BLOCK_SIZE+3]<<8));
        // scan for strikes, if level is exceeded then
        // register new strike, starting data window LEAD samples earlier
        // than trigger sample
        power=sqrt(((double)left*(double)left)+((double)right*(double)right));
        avgpower.Add(power);
        if (blocksread>200 && power>GetTriggerLevel()) {
          strikes.NewStrike(scanblock+BLOCK_SIZE+i-(4*STRIKE_LEAD_SAMPLES),BLOCK_SIZE-i+(4*STRIKE_LEAD_SAMPLES),clk+i,GetTriggerLeveldB());
          i+=((STRIKE_GUARD_SAMPLES-1)*4); // guard time, next strike cannot be closer than this
        }
      }
      // scanned data, now move it to past buffer
      // TODO: performance improvement possible here by swapping blocks instead of copy
      memcpy(scanblock,scanblock+BLOCK_SIZE,BLOCK_SIZE);
    }
  }
}
