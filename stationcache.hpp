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
#ifndef __stationcache_hpp__
#define __stationcache_hpp__

#include "ligrid.hpp"

// each connected station sends information about other stations that
// it knows to its peers. station cache keeps this information
// for up to 100 stations. If new entry arrives and cache
// is already full, then oldest existing entry is replaced
//
// TODO: should expire stale entries based on age
class CStationCache : public CPointerQueue, public CThread
{

  typedef struct {
    time_t updatetime;
    IPADDR adr;
    double longitude, latitude;
    char *name;
    char *announcement;
    CTCPConnection *connection;
    CTimer connectiontimer;
    
    void Dump() {
      Log.Write(" updatetime      : %d",updatetime);
      Log.Write(" address         : %s",(const char*)adr);
      Log.Write(" name            : %s",name);
      Log.Write(" announcement    : %s",announcement);
      Log.Write(" connection      : %s",connection?"Connected":"Not connected");
      Log.Write(" connectiontimer : %d ms left",connectiontimer.RemainingMs()); 
    }
    
  } STATION;

STATION cache[32];
CMutex mutex;

  void AdvertiseMe();
  void DiscardEntry(int index);
  
public:
  CStationCache();
  ~CStationCache();
  void *Run(void *arg);
  void UpdateCache(IPADDR& adr,double longitude,double latitude,char *name,char *announcement);
  void StatusPage(CTCPConnection& c);
  void CheckInput(const char *s,int len,char *remoteaddr=NULL);
  void Load(const char *filename);
  void Store(const char *filename);
  void AdvertiseStation(int index);
  void SendToUI(CTCPConnection& connection);
};

#endif
