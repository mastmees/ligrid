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

// Connection listener and connection pool for other clients who
// want to triangulate.
//
// Listener runs in its own thread
//
class CClientConnectionPool : public CConnectionPool
{
public:
  CClientConnectionPool(int maxc=32):CConnectionPool(maxc) {}

  virtual void onClientData(CTCPConnection& connection,char *buf,int len)
  {
    Log.Write("%s : %*.*s",(char *)connection.remoteaddr,len,len,buf);
    if (*buf=='#') {
      switch (buf[1]) {
        case 'q':
        case 'Q':
          Log.Write("Closing connection to %s",(char *)connection.remoteaddr);
          connection.Close();
          break;
      }
    }
    else {
      stationcache->CheckInput(buf,len,connection.remoteaddr);
    }
  }
};


CListener::CListener()
{
  config.Lock();
  connectionpool=new CClientConnectionPool(config.GetIntSetting("maxconnections",32));
  config.Unlock();
}

CListener::~CListener()
{
  delete connectionpool;
}

void * CListener::Run(void *arg)
{
CTCPConnection* connection;
char s[512],d[128];
  Log.Write("Connection listener starting");
  if (listener.CreateListener(config.GetIntSetting("serverport",4711),8)) {
    while (!Quitting()) {
      connection=new CTCPConnection;
      if (listener.Accept(*connection)) {
        Log.Write("Incoming connection from %s",(char *)connection->remoteaddr);
        GetTime(d);
        sprintf(s,"#O%s %s,%s,%s,%s,%s,%08u\r\n",d,
          config.GetStringSetting("stationannouncement","Connected to LiGrid"),
          config.GetStringSetting("stationname",""),
          config.GetStringSetting("longitude",""),
          config.GetStringSetting("latitude",""),
          VERSIONSTRING,
          (uint)sampler.GetClockDrift());
        // TODO: find out if this is correct, and what the client should do with the drift value 
        connection->Write(s,strlen(s),MSG_DONTWAIT);
        connectionpool->AddConnection(connection);
      }
    }
    Log.Write("clientconnectionlistener quitting");
    listener.Close();
  }
  else
    Log.Write("CreateListener failed: %s",strerror(errno));
  Log.Write("Connection listener stopped");
  return NULL;
}


