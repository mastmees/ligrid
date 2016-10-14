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

#define CACHEFILENAME 		"ligrid.cache"
#define ADVERTISINGPERIOD 	60   // look for new non-connected nodes this often
#define CONNECTIONTIMER		300  // once connected, stay connected for this many seconds
#define RECHECKTIMER        1800 // re-verify after this many seconds

void * CStationCache::Run(void *arg)
{
CTimer checktimer(ADVERTISINGPERIOD*1000),storetimer(3600*1000);
int aindex=0,i;
  while (!Quitting()) {
    if (checktimer.Expired()) {
      int c=0;
      checktimer.Set(ADVERTISINGPERIOD*1000);
      for (i=0;i<COUNTOF(cache) && !Quitting();i++) {
        if (cache[aindex].updatetime) {
          if (cache[aindex].adr.GetAddr()==0 && cache[aindex].connectiontimer.Expired()) { // my own record
            AdvertiseStation(aindex);
            cache[aindex].connectiontimer.Set(60*1000);
          }
          if (!c && cache[aindex].connectiontimer.Expired() && !cache[aindex].connection) {  // non-connected node and not checked any this time
            cache[aindex].connection=new CTCPConnection;
            if (cache[aindex].connection) {
              cache[aindex].connection->remoteaddr=cache[aindex].adr;
              Log.Write("Trying to connect to %s (%s)",(const char *)cache[aindex].adr,cache[aindex].name);
              if (cache[aindex].connection->Open()) {
                Log.Write("Connected to %s (%s)",(const char*)cache[aindex].adr,cache[aindex].name);
                cache[aindex].updatetime=time(NULL);
                cache[aindex].connectiontimer.Set(CONNECTIONTIMER*1000);
                if (!Quitting())
                AdvertiseStation(aindex);
                c++;
              }
              else {
                Log.Write("Removing dead node %s (%s) from cache",(const char *)cache[aindex].adr,cache[aindex].name);
                DiscardEntry(aindex); // quickly kill off dead nodes
              }
            }
            else {
              Log.Write("Could not create CTCPConnection");
            }
          }
          else {
            if (cache[aindex].connection && cache[aindex].connectiontimer.Expired()) {
              Log.Write("Closing connection to %s (%s)",(const char *)cache[aindex].adr,cache[aindex].name);
              cache[aindex].connection->Close();
              delete cache[aindex].connection;
              cache[aindex].connection=NULL;
              cache[aindex].connectiontimer.Set(RECHECKTIMER*1000);
            }
          }
        }
        aindex=(aindex+1)%COUNTOF(cache);
      }
    }
    if (storetimer.Expired()) {
      Store(CACHEFILENAME);
      storetimer.Set(3600*1000);
    }
    if (!Quitting())
      Sleep(500);
  }
  return NULL;
}

CStationCache::CStationCache()
{
  memset(cache,0,sizeof(cache));
  Log.Write("Created station cache for %d stations",COUNTOF(cache));
  Load(CACHEFILENAME);
  AdvertiseMe();
}

CStationCache::~CStationCache()
{
  Stop();
  Store(CACHEFILENAME);
  for (int i=0;i<COUNTOF(cache);i++) {
    DiscardEntry(i);
  }
  Log.Write("Station cache deleted");
}

void CStationCache::SendToUI(CTCPConnection& connection)
{
char buf[1024];
  mutex.Lock();
  for (int i=0;i<COUNTOF(cache);i++) {
    if (cache[i].updatetime) {
      sprintf(buf,"#R%s,%.5f,%.5f,%s\r",
        (const char*)cache[i].adr,cache[i].latitude,
        cache[i].longitude,cache[i].name);
      connection.Write(buf,strlen(buf));
    }
  }
  mutex.Unlock();
}

void CStationCache::AdvertiseMe()
{
  char buf[8192];
  GetTime(buf);
  config.Lock();
  sprintf(strchr(buf,'\0'),",0.0.0.0,%s,%s,%s,%s,#O\r",
          config.GetStringSetting("latitude",""),
          config.GetStringSetting("longitude",""),
          config.GetStringSetting("stationname",""),
          config.GetStringSetting("stationannouncement",""));
  config.Unlock();
  CheckInput(buf,strlen(buf),NULL); // insert/update myself in cache
}

void CStationCache::AdvertiseStation(int index)
{
  char buf[8192],a[64];
  uint addr=0;
  GetTime(buf);
  mutex.Lock();
  if (cache[index].updatetime) {
    Log.Write("Advertising station %s (%s)",(const char*)cache[index].adr,cache[index].name);
    addr=cache[index].adr.GetAddr();
    strcpy(a,(const char*)cache[index].adr);
    *strchr(a,':')='\0';
    sprintf(strchr(buf,'\0'),"%s,%.5f,%.5f,%s,%s,#O\r",
          a,cache[index].latitude,cache[index].longitude,
          cache[index].name,cache[index].announcement);
    for (int i=0;i<COUNTOF(cache);i++) {
      // advertise to open connections, except to the same one that we advertise out
      if (cache[i].connection && cache[i].adr.GetAddr()!=addr) {
        if (cache[i].connection->Write(buf,strlen(buf),MSG_DONTWAIT)<0) {
          DiscardEntry(i);
        }
      }
    }
  }
  mutex.Unlock();
}

void CStationCache::DiscardEntry(int index)
{
  if (cache[index].connection) {
    cache[index].connection->Close();
    delete cache[index].connection;
  }
  if (cache[index].announcement)
    delete cache[index].announcement;
  if (cache[index].name)
    delete cache[index].name;
  cache[index].connection=NULL;
  cache[index].announcement=NULL;
  cache[index].name=NULL;
  cache[index].updatetime=0;
}

void CStationCache::UpdateCache(IPADDR& adr,double longitude,double latitude,char *name,char *announcement)
{
  if (!name) name=(char *)"";
  if (!announcement) announcement=(char *)"";
  mutex.Lock();
  int i,j=-1,o=-1;
  for (i=0;i<COUNTOF(cache);i++) {
    if (!cache[i].updatetime) {
      if (j<0)
        j=i; // first free slot
    }
    else {
      if (o<0 || cache[i].updatetime<cache[o].updatetime)
        o=i; // index to oldest entry
      if (adr.GetAddr()==cache[i].adr.GetAddr() || (longitude==cache[i].longitude && latitude==cache[i].latitude)) {
        cache[i].updatetime=time(NULL);
        cache[i].longitude=longitude;
        cache[i].latitude=latitude;
        cache[i].adr=adr;
        cache[i].connectiontimer.Set(0);
        if (cache[i].name)
          delete cache[i].name;
        if (cache[i].announcement)
          delete cache[i].announcement;
        cache[i].name=new char[strlen(name)+1];
        if (cache[i].name)
          strcpy(cache[i].name,name);
        config.rtrim(cache[i].name);
        cache[i].announcement=new char[strlen(announcement)+1];
        if (cache[i].announcement)
          strcpy(cache[i].announcement,announcement);
        config.rtrim(cache[i].announcement);
        Log.Write("updated station %s information in cache",(const char*)adr);
        mutex.Unlock();
        return;
      }
    }
  }
  // did not exist in cache
  if (j<0) {
    // cache full, discard oldest entry
    if (cache[o].announcement)
      delete cache[o].announcement;
    if (cache[o].connection) {
      cache[o].connection->Close();
      delete cache[o].connection;
    }
    j=o;
  }
  cache[j].updatetime=time(NULL);
  cache[j].longitude=longitude;
  cache[j].latitude=latitude;
  cache[j].adr=adr;
  cache[j].announcement=new char[strlen(announcement)+1];
  cache[j].name=new char[strlen(name)+1];
  cache[j].connection=NULL;
  cache[j].connectiontimer.Set(0);
  if (cache[j].name)
    strcpy(cache[j].name,name);
  config.rtrim(cache[j].name);
  if (cache[j].announcement)
    strcpy(cache[j].announcement,announcement);
  config.rtrim(cache[j].announcement);
  Log.Write("added station %s information to cache",(const char*)adr);
  mutex.Unlock();
}

// station cache renders its own part of status page
//
void CStationCache::StatusPage(CTCPConnection& c)
{
  char *s="<h1>Station cache</h1>\n<table border=1><tr><td>IP</td><td>Connected</td><td>Longitude&nbsp;&nbsp;&nbsp;</td><td>Latitude&nbsp;&nbsp;&nbsp;</td><td>Station</td><td width=30%>Message</td></tr>\n";
  int i;
  char buf[128];
  c.Write(s,strlen(s));
  mutex.Lock();
  for (i=0;i<COUNTOF(cache);i++) {
    if (cache[i].updatetime) {
      sprintf(buf,"<tr><td>%s</td><td>%s</td><td>%.5f</td><td>%.5f</td><td>",
        (char *)cache[i].adr,(cache[i].connection && cache[i].connection->GetSd()!=-1)?"yes":"",
        cache[i].longitude,cache[i].latitude);
      c.Write(buf,strlen(buf));
      if (cache[i].name)
        c.Write(cache[i].name,strlen(cache[i].name));
      s="</td><td>";
      c.Write(s,strlen(s));
      if (cache[i].announcement)
        c.Write(cache[i].announcement,strlen(cache[i].announcement));
      s="</td></tr>\n";
      c.Write(s,strlen(s));
    }
  }
  mutex.Unlock();
  s="</table>\n";
  c.Write(s,strlen(s));
}

void CStationCache::Load(const char *filename)
{
FILE *fp;
char buf[512];
  Log.Write("Loading station cache from %s",filename);
  if ((fp=fopen(filename,"rt"))!=NULL) {
    while (fp && !feof(fp) && !ferror(fp)) {
      if (fgets(buf,sizeof(buf),fp)!=NULL)
        CheckInput(buf,strlen(buf),NULL);    
    }
    fclose(fp);
  }
}

void CStationCache::Store(const char *filename)
{
FILE *fp;
char d[128],a[64];
  Log.Write("Storing station cache to %s",filename);
  GetTime(d);
  mutex.Lock();
  if ((fp=fopen(filename,"wt"))!=NULL) {
    for (int i=0;i<COUNTOF(cache);i++) {
      if (cache[i].updatetime) {
        strcpy(a,(const char *)cache[i].adr);
        *strchr(a,':')='\0';
        fprintf(fp,"%s,%s,%.5f,%.5f,%s,%s,#O\n",d,a,cache[i].latitude,cache[i].longitude,
          cache[i].name,cache[i].announcement);
      }
    }
    fclose(fp);
  }
  mutex.Unlock();
}

// Check a line for cache update command (#O as last field) and update
// cache if it is
//
//2009/03/15 09:37:15,0.0.0.0,24.65278,059.39500,Lauri Tallinn,,#O 
//2009/03/15 14:15:03,0.0.0.0,52.25056,006.80556,Hengelo the Netherlands,82.75.173.138  or  cc199994-a.hnglo1.ov.home.nl,#O
//2009/03/15 13:50:42,78.145.113.120,53.06748,-002.04249, Cheddletom,,#O
//
void CStationCache::CheckInput(const char *s,int len,char *remoteaddr)
{
  char *t=(char *)s,*tt,*a;
  char x[512];
  double longitude,latitude;
  IPADDR adr;
  if (!s || len>511)
    return;
  while (t && strchr(t,',')) {
    t=strchr(t,',')+1;
  }
  if (t && strstr(t,"#O") || strstr(t,"#o")) {
    memcpy(x,s,len);
    x[len]='\0';
    t=strchr(x,',');
    if (t) {
      tt=t+1;
      t=strchr(tt,',');
      if (t)
        *t='\0';
      adr.SetAddr(tt);
      if (adr==0) {
        if (remoteaddr)
          adr.SetAddr(remoteaddr);
      }
      if (adr!=0)
        adr.SetPort(4711);
      if (t) {
        t++;
        latitude=strtod(t,NULL);
        t=strchr(t,',');
        if (t) {
          t++;
          longitude=strtod(t,NULL);
          t=strchr(t,',');
          if (t) {
            t++;
            tt=strchr(t,',');
            if (tt) {
              *tt='\0';
              tt++;
              a=tt;
              tt=strchr(tt,',');
              if (tt)
                *tt='\0';
              UpdateCache(adr,longitude,latitude,config.skipspace(t),config.skipspace(a));
            }
          }
        }
      }
    }
  }
}
