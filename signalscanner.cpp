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

CSignalScanner signalscanner;

// send HUP signal to myself if restart is needed
//
void CSignalScanner::restartcallback(const char *n,const char *v)
{
  kill(getpid(),SIGHUP);
}

// trigger parameter changes are applied on the fly to sampler
//
void CSignalScanner::changetriggercallback(const char *n,const char *v)
{
  if (!strcasecmp(n,"triggerlevel")) {
    if (!strcasecmp(v,"auto")) {
      Log.Write("Automatic trigger level enabled");
      sampler.SetAutoTriggerLevel(sampler.GetAutoTriggerLevel());
    }
    else {
      double level=strtod(v,NULL);
      Log.Write("Setting detection trigger level to %f dB",level);
      sampler.SetAutoTriggerLevel(0.0);
      sampler.SetTriggerLeveldB(level);
    }
  } else if (!strcasecmp(n,"autotriggerratio")) {
    double ratio=strtod(v,NULL);
    Log.Write("Setting auto trigger ratio to %f",ratio);
    sampler.SetAutoTriggerLevel(ratio);
  }
}

void CSignalScanner::invertleftcallback(const char *n,const char *v)
{
  bool b=(*v=='y' || *v=='Y' || *v=='t' || *v=='T' || atoi(v));
  sampler.SetOptions(b,sampler.GetSwapChannels());
  Log.Write("Inverting left channel %s",b?"enabled":"disabled");
}

void CSignalScanner::swapchannelscallback(const char *n,const char *v)
{
  bool b=(*v=='y' || *v=='Y' || *v=='t' || *v=='T' || atoi(v));
  sampler.SetOptions(sampler.GetInvertLeft(),b);
  Log.Write("Swapping left and right channels %s",b?"enabled":"disabled");
}

void *CSignalScanner::Run(void *arg)
{
  config.Lock();
  const char *input=config.GetStringSetting("recsource","line");
  CStereoInput::RECORDINGSOURCE iinput=CStereoInput::LINE;
  if (!strcasecmp(input,"line1"))
    iinput=CStereoInput::LINE1;
  else if (!strcasecmp(input,"line2"))
    iinput=CStereoInput::LINE2;
  else if (!strcasecmp(input,"line3"))
    iinput=CStereoInput::LINE3;       
  int volume=config.GetIntSetting("volume",20);
  int rate=config.GetIntSetting("samplerate",48000);
  const char *device=config.GetStringSetting("device",RECORDING_DEVICE);
  if (sampler.Open(volume,device,rate,iinput)) {
    Log.Write("Audio device %s opened, sampling %s at %.2f kHz, volume %d",device,input,(double)rate/1000.0,volume);
    input=config.GetStringSetting("triggerlevel","");
    if (!strcasecmp(input,"auto")) {
      Log.Write("Automatic trigger level enabled");
      sampler.SetAutoTriggerLevel(config.GetDoubleSetting("autotriggerratio",8.0));
    }
    else {
      double level=config.GetDoubleSetting("triggerlevel",-50.0);
      Log.Write("Setting detection trigger level to %f dB",level);
      sampler.SetAutoTriggerLevel(0.0);
      sampler.SetTriggerLeveldB(level);
    }
    sampler.SetOptions(config.GetBoolSetting("invertleft",false),
      config.GetBoolSetting("swapchannels",false));
    config.Unlock();
    config.RegisterListener("device",this->restartcallback);
    config.RegisterListener("recsource",this->restartcallback);
    config.RegisterListener("volume",this->restartcallback);
    config.RegisterListener("triggerlevel",this->changetriggercallback);
    config.RegisterListener("autotriggerratio",this->changetriggercallback);
    config.RegisterListener("invertleft",this->invertleftcallback);
    config.RegisterListener("swapchannels",this->swapchannelscallback);
    while (!Quitting()) {
      sampler.ReadBlock();
      sampler.ProcessData();
    }
    sampler.Close();
  }
  else
    config.Unlock();
  return NULL;
}

