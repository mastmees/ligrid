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
#ifndef __filter_hpp__
#define __filter_hpp__

// Implements a FIR digital filter of arbitrary length
// once initialized with coefficent table, just call the
// Filter() method with new samples, and get filtered sample
// values back
//
// filter delay line is initalized to zeroes, so the output will be
// no good until number of samples equal to number of filter coefficents
// has passed through
//
class FIR
{
double *delayline;
double *coeff;
int len,count;

public:

  FIR(int ncoeff,double *fcoeff)
  {
    delayline=new double[ncoeff*sizeof(double)];
    coeff=new double[ncoeff*sizeof(double)];
    len=ncoeff;
    count=0;
    memcpy(coeff,fcoeff,ncoeff*sizeof(double));
    memset(delayline,0,ncoeff*sizeof(double));
  }
  
  ~FIR()
  {
    delete delayline;
    delete coeff;
  }
  
  double Filter(double sample)
  {
    double r=0.0;
    int index=count;
    delayline[count]=sample;
    for (int j=0;j<len;j++) {
      r=r+coeff[j]*delayline[index];
      index--;
      if (index<0) index=len-1;
    }
    count++;
    if (count>=len) count=0;
    return r;
  }
};

#endif
