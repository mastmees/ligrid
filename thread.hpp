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
#ifndef __tasking_hpp__
#define __tasking_hpp__

#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// simple threading and timing primitives - timer, mutex, and thread
//

// you create and initialize timer for number of milliseconds, then
// call Expired() or RemainingMs() at any time to check the state
//
class CTimer
{
public:
  CTimer(unsigned timeoutms=0)
  {
    Set(timeoutms);
  }
  
  ~CTimer()
  {
  }
  
  void Set(unsigned timeoutms)
  {
    gettimeofday(&exptime,NULL);
    exptime.tv_sec+=timeoutms/1000;
    exptime.tv_usec+=(timeoutms%1000)*1000;
    while (exptime.tv_usec>999999) {
      exptime.tv_sec++;
      exptime.tv_usec-=1000000;
    }
  }
  bool Expired()
  {
    return !RemainingMs();
  }
  
  uint RemainingMs()
  {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    long l;
    l=(exptime.tv_sec-tv.tv_sec)*1000+(exptime.tv_usec-tv.tv_usec)/1000;
    return (l>0?l:0);
  }
  
private:
  struct timeval exptime;
};

// simple wrapper class around pthread mutex
//
class CMutex
{
  pthread_mutex_t mutex;
  pthread_mutexattr_t attr;
public:
  CMutex() {
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
    if (pthread_mutex_init(&mutex,&attr))
    {
      perror("CMutex::CMutex()");
      *(int *)0=0;
    }
  }
  ~CMutex() {
    if (pthread_mutex_destroy(&mutex))
      perror("CMutex::~CMutex()"); 
  }
  void Lock() {
    if (pthread_mutex_lock(&mutex)) {
      perror("CMutex::Lock()"); 
      *(int *)0=0;
    }
  }
  void Unlock() {
    if (pthread_mutex_unlock(&mutex))
      perror("CMutex::Unlock()");
  }
};
 
// to create a new thread, inherint from CThread and implement
// your own Run() method for derived class. Then call Start()
// to execute your Run() in separate thread.
//
// If you are not interested of exit status of the thread, then
// use Detach() to avoid resource leaking, once detached thead
// becomes completely independent and you cant get its result
// code any more
//
// If you want to wait for thread completion and result, then dont
// detach and use Wait() instead
//
// You can stop a running thread with Stop(), it will wait 10 
// seconds for Run() method to exit gracefully and if that does not
// happen will forcefully kill the thread. For graceful exit,
// poll the thread status with Quitting() method periodically and
// return from method if true is returned 
//
class CThread
{
pthread_t thread;
void *arg;
const char *name;
int quitting;
int running;

  static void* Runner(void *arg)
  {
    int i;
    void *ret;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&i);
    ret=((CThread*)arg)->Run(((CThread*)arg)->arg);
    ((CThread*)arg)->quitting=2;
    return ret;
  }

public:

  bool Detach()
  {
    return pthread_detach(thread)==0;
  }
    
  void Start(const char *name,void *arg=NULL)
  {
    this->arg=arg;
    this->name=name;
    this->quitting=false;
    this->running=1;
    pthread_create(&thread,NULL,Runner,(void *)this);
  }
  
  void* Wait()
  {
    void *retval=NULL;
    if (running) {
      pthread_join(thread,&retval);
      running=0;
    }
    return retval;
  }
  
  void Sleep(uint sleepms)
  {
    CTimer t(sleepms);
    uint r;
    while (running && !quitting && (r=t.RemainingMs())>0)
      usleep(r>10000?10000:r);
  }
  
  void Stop()
  {
    if (running && !quitting) {
      quitting=1;
      for (int i=0;i<100;i++) {
        if (quitting!=1)
          return;
        usleep(10000);
      }
    }
    if (running)
      pthread_cancel(thread);
    running=0;
  }

  bool Quitting()
  {
    return (!running || quitting);
  }
  
  virtual ~CThread()
  {
    if (quitting!=2)
      Stop();
    Wait();
  }
    
  virtual void* Run(void *arg)=0;
};

#endif
