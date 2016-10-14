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

// a simple IP address based authentication is used to
// check if station is authorized to change settings
//
class CAdminAuth
{
private:

  typedef struct _authentry {
    IPADDR adr;
    uint mask;
  } AUTHENTRY;
  
  AUTHENTRY authentries[32];
  int nentries;

public:

  CAdminAuth()
  {
    nentries=0;
    uint p,mask;
    IPADDR adr;
    const char *s=config.GetStringSetting("adminstations","");
    if (*s) {
      Log.Write("adminstations=%s",s);
      char *ss,*ot=new char[strlen(s)+1];
      char *t=ot;
      if (t) {
        strcpy(t,s);
        while (nentries<COUNTOF(authentries) && *t) {
          p=32;
          if (adr.SetAddr(t)) {
            ss=strchr(t,',');
            if (ss) {
              Log.Write("Parsing %s",t);
              *ss='\0';
              ss=strchr(t,'/');
              if (ss)
                p=atoi(ss+1);
              t=strchr(t,'\0')+1;
            }
            else {
              Log.Write("Parsing %s",t);
              ss=strchr(t,'/');
              if (ss)
                p=atoi(ss+1);
              t=strchr(t,'\0');
            }
            mask=(((uint)pow(2,p))-1)<<(32-p);
            Log.Write("Adding admin station %s/%d mask %08x",(char *)adr,p,mask);
            authentries[nentries].adr=adr;
            authentries[nentries].mask=mask;
            nentries++;
          }
          else
            break;
        }
        delete ot;
      }
    }
  }
  
  bool isAdminStation(CTCPConnection& connection)
  {
    for (int i=0;i<nentries;i++) {
      if ( (((uint)connection.remoteaddr)&authentries[i].mask) == 
                  (((uint)authentries[i].adr)&authentries[i].mask)) {
        Log.Write("%s is authorized",(char *)connection.remoteaddr);
        return true;
      }  
    }
    Log.Write("%s is not authorized",(char *)connection.remoteaddr);
    return false;
  }  

};

class CUIConnectionPool : public CConnectionPool
{
private:

CAdminAuth adminstations;
  
public:
  CUIConnectionPool(int maxc=32):CConnectionPool(maxc)
  {
  }
  
  ~CUIConnectionPool()
  {
  }

  // check what  the connected UI has sent
  // UI can modify settings with #S if its IP address is in admin address list
  // otherwise it can just gracefully close the connection
  // by sending #Q, or send station cache updates
  //
  virtual void onClientData(CTCPConnection& connection,char *buf,int len)
  {
    char *s,*t;
    Log.Write("%s : %*.*s",(char *)connection.remoteaddr,len,len,buf);
    if (*buf=='#') {
      switch (buf[1]) {
        case 'r': // remote stations
        case 'R':
          Log.Write("Sending station cache to %s",(const char*)connection.remoteaddr);
          stationcache->SendToUI(connection);
          break;
        case 'q': // disconnect
        case 'Q':
          Log.Write("Closing connection to %s",(char *)connection.remoteaddr);
          connection.Close();
          break;
        case 's': // setting
        case 'S':
          s=strchr(buf+2,'=');
          if (s) {
            *s++='\0';
            // check if the station is allowed to modify settings
            if (adminstations.isAdminStation(connection) && strcasecmp(buf+2,"adminstations")) {
              Log.Write("%s modifying config: %s=%s",(char *)connection.remoteaddr,
                buf+2,s);
              config.SetValue(buf+2,s);
            }
          }
          config.Lock();
          // dont allow modifying admin stations remotely
          if (strcasecmp(buf+2,"adminstations")) {
            if (!s)
              s=(char *)config.GetStringSetting(buf+2,"");
          }
          else
            s="";
          t=new char[strlen(buf+2)+strlen(s)+10];
          if (t) {
            sprintf(t,"#S%s=%s\r",buf+2,s);
            connection.Write(t,strlen(t));
            delete t;
          }
          config.Unlock();
          break;
      }
    }
    else
      stationcache->CheckInput(buf,len,(char *)connection.remoteaddr);
  }
};

CUIListener::CUIListener()
{
  config.Lock();
  connectionpool=new CUIConnectionPool(config.GetIntSetting("maxuiconnections",32));
  config.Unlock();
}

CUIListener::~CUIListener()
{
  delete connectionpool;
}

// UI listener accepts incoming connections, responds with 
// station announcement and puts connections to ui connection pool
// to which lightning strike data will be broadcasted
//
void *CUIListener::Run(void *arg)
{
CTCPConnection* connection;
char s[512],d[128];
  Log.Write("UI Connection listener starting");
  config.Lock();
  if (listener.CreateListener(config.GetIntSetting("uiserverport",4713),8)) {
    config.Unlock();
    while (!Quitting()) {
      connection=new CTCPConnection;
      if (listener.Accept(*connection)) {
        Log.Write("Incoming UI connection from %s",(char *)connection->remoteaddr);
        GetTime(d);
        config.Lock();
        sprintf(s,"#B%s %s,%s,%s,%s,%s,%08u\r\n",d,
          config.GetStringSetting("stationannouncement","Connected to LiGrid"),
          config.GetStringSetting("stationname",""),
          config.GetStringSetting("longitude",""),
          config.GetStringSetting("latitude",""),
          VERSIONSTRING,
          (uint)sampler.GetClockDrift());
        config.Unlock();
        connection->Write(s,strlen(s),MSG_DONTWAIT);
        connectionpool->AddConnection(connection);
      }
    }
  }
  else {
    config.Unlock();
    Log.Write("CreateListener failed: %s",strerror(errno));
  }
  Log.Write("Connection listener stopped");
  return NULL;
}

