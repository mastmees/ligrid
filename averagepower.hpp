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
#ifndef __averagepower_hpp__
#define __averagepower_hpp__

// Tracks average value over given number of samples
//
class CAveragePower {

double *samples;
int idx,count;
double value;

public:

  CAveragePower(int nsamples=400)
  {
    if (nsamples<=0)
      nsamples=1;
    idx=0;
    count=nsamples;
    value=0.0;
    samples=new double[nsamples];
    while (nsamples)
      samples[--nsamples]=0.0;
  }
  
  ~CAveragePower()
  {
    delete samples;
  }
  
  double Get()
  {
    return value/(double)count;
  }
  
  double Add(double sample)
  {
    value+=sample;
    value-=samples[idx];
    samples[idx]=sample;
    idx=(idx+1)%count;
    return value/(double)count;
  }
  
};

#endif
