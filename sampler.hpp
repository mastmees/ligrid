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
#ifndef __sampler_hpp__
#define __sampler_hpp__

#include "pointerqueue.hpp"
#include "averagepower.hpp"
#include "ligrid.hpp"

// sampler has two parts - the signal sampler that captures raw data,
// applies filters and preprocessing, then posts the captured data to
// data processing part using pointer queue.
// data processor scans the signal for lightning strikes, and if one
// is found then registers it, possibly waiting for next block to collect
// 200 samples before analysis.
//
class CSampler {

private:
int ifd;						// recording device file descriptor
uchar block[BLOCK_SIZE*2];		// data block buffer, twice the size
CPointerQueue blockqueue;   	// data block queue between Read&Process
uchar scanblock[BLOCK_SIZE*2];  // two-block buffer, for past and current
double triggerlevel;			// fixed trigger power level as set
double autotriggerlevel;		// automatic trigger level multiplier
CAveragePower avgpower;			// power average over N samples
long clock;						// sampler clock at start of current block
struct timeval tv;				// system timestamp at a time the clock set
int invertleft,swapchannels;	// options for wiring correction
unsigned long blocksread;		// total number of blocks processed
CStrikes strikes;				// strike data being collected
CMutex mutex;					// internally used mutex
FIR *left,*right;				// FIR filters for left&right channels
FIR *leftlp,*rightlp;           // anti-alias filters for downsampling
int samplerate;                 // hardware sampling rate
int firstblockread;             // has the first block been read?
struct timeval first_tv;        // if yes, then this is time after getting it
unsigned long realrate;			// how many samples per day we really get

  void Init();
public:
  uchar *GetBlock() { return block; }
  void SetOptions(int invertleft,int swapchannels);
  bool GetInvertLeft() { return invertleft; }
  bool GetSwapChannels() { return swapchannels; }
  long GetClock();
  unsigned long GetClockDrift(); // real samples per day/22.05
  CSampler();
  ~CSampler();
  bool Open(int volume=50,const char *device=RECORDING_DEVICE,
      int samplerate=48000,
      CStereoInput::RECORDINGSOURCE input=CStereoInput::LINE);
  void Close();
  int ReadBlock();
  bool AddData(uchar *data,uint nbytes,long c);
  void SetAutoTriggerLevel(double multiplier);
  double GetAutoTriggerLevel() { return autotriggerlevel; }
  void SetTriggerLeveldB(double leveldB);
  double GetTriggerLevel(); // changes dynamically in auto mode
  double GetTriggerLeveldB(); // get level in dB, changes dynamically in auto
  double GetTriggerLevelSetting() { return triggerlevel; } // returns set value
  void SetTriggerLevel(double level);
  void ProcessData();
};

extern CSampler sampler;

#endif

