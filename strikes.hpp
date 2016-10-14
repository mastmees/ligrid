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
#ifndef __strikes_hpp__
#define __strikes_hpp__

#include "ligrid.hpp"

// a collection of lightning strikes where data samples are collected
// until required amount for analysis is available. Its actually an overkill
// because there should not be more than one incomplete at any time
//
class CStrikes
{
  typedef struct {
    int nbytes;  	// number of bytes in samples
    long clock; 	// first sample timestamp
    double level; 	// trigger level at a time of occurrance, dB value
    uchar samples[STRIKE_WINDOW_SIZE*4]; // little-endian 16 bit samples
  } STRIKE;
  
  STRIKE strikes[32];
  uint nstrikes;
  
public:

  CStrikes()
  {
    memset(strikes,0,sizeof(strikes));
    nstrikes=0;
  }

  // new strike detected. data points to start of
  // sample window, nbytes tells how many bytes are
  // available at this time
  // clk is a timestamp of first sample in window
  bool NewStrike(uchar *data,uint nbytes,unsigned long clk,double triglevel)
  {
    STRIKE s;
    if (nbytes>=sizeof(s.samples)) { // easy case, data is available for entire window
      memcpy(s.samples,data,sizeof(s.samples));
      s.nbytes=sizeof(s.samples);
      s.clock=clk;
      s.level=triglevel;
      StrikeDetected(&s);
    }
    else { // more complicated, keep as much data as there is and wait for more
      if (nstrikes>=COUNTOF(strikes))
        return false;
      strikes[nstrikes].nbytes=nbytes;
      memcpy(strikes[nstrikes].samples,data,nbytes);
      strikes[nstrikes].clock=clk;
      strikes[nstrikes].level=triglevel;
      nstrikes++;
    }
    return true;
  }

  // new block of samples available, finish up partial strikes
  // when done, no more partial strikes will remain
  void FinishStrikes(uchar *data,int nbytes)
  {
    uint i;
    for (i=0;i<nstrikes;i++) {
      memcpy(strikes[i].samples+strikes[i].nbytes,data,
        sizeof(strikes[i].samples)-strikes[i].nbytes);
      strikes[i].nbytes=sizeof(strikes[i].samples);
      StrikeDetected(&strikes[i]);
    }
    nstrikes=0;
  }
  
  // implement this somewhere, NewStrike() and FinishStrikes() will
  // call this method for all strikes that have complete dataset
  //
  void StrikeDetected(STRIKE *s);
  
};

#endif
