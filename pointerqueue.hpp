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
#ifndef __pointerqueue_hpp__
#define __pointerqueue_hpp__

#include <stdlib.h>
#include "thread.hpp"

// queue (FIFO buffer) for finite number of void pointers
// Add() inserts a new entry to queue
// Get() returns an oldest entry and removes the pointer from queue
// calling Get() on empty queue returns NULL
// yes, I know stl has this, and a kitchen sink too
//
class CPointerQueue
{
protected:
  void **queue;
  int head,tail,count,len;
  CMutex mutex;
    
public:
  CPointerQueue(int size=32)
  {
    queue=new void*[size];
    len=size;
    head=tail=count=0;
  }
  
  ~CPointerQueue()
  {
    if (queue)
      delete(queue);
  }
  
  bool Add(void *p)
  {
    if (count<len) {
      mutex.Lock();
      queue[head]=p;
      head=(head+1)%len;
      count++;
      mutex.Unlock();
      return true;
    }
    return false;
  }
  
  int Size()
  {
    return len;
  }
  
  int Count()
  {
    return count;
  }
  
  void *Get()
  {
    void *s;
    if (count) {
      mutex.Lock();
      s=queue[tail];
      tail=(tail+1)%len;
      count--;
      mutex.Unlock();
      return s;
    }
    return NULL;
  }
  
};

#endif
