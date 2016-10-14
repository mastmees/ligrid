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

class CWebConnectionPool : public CConnectionPool , public CThread
{
public:
  CWebConnectionPool(int maxc=32):CConnectionPool(maxc) {}

  void *Run(void *arg)
  {
    while (!Quitting()) {
      Poll();
      usleep(10000);      
    }
    return NULL;
  }

  // no matter what the client actually asks for, we are replying with
  // HTTP 1.1 response and close connection
  // the response is statuspage of LiGrid,
  // showing connections for all connection pools and also
  // current contents of station cache
  //  
  virtual void onClientData(CTCPConnection& connection,char *buf,int len)
  {
    char *message="HTTP/1.1 200 OK\n"
	  "Content-type: text/html; charset=UTF-8\n"
      "Connection: close\n"
      "Server: localhost\n"
      "\n<html><head><title>LiGrid status</title></head><body>\n";
    connection.Write((void *)message,strlen(message));
    message="<h1>Connected clients</h1>\n";
    connection.Write((void *)message,strlen(message));
    connectionlistener->connectionpool->StatusPage(connection);
    message="<h1>Connected user interfaces</h1>\n";
    connection.Write((void *)message,strlen(message));
    uiconnectionlistener->connectionpool->StatusPage(connection);
    stationcache->StatusPage(connection);
    message="</body></html>\n";
    connection.Write((void *)message,strlen(message));
    connection.Close();
  }
};

CWebListener::CWebListener()
{
  config.Lock();
  connectionpool=new CWebConnectionPool(config.GetIntSetting("maxwebconnections",32));
  connectionpool->Start("webconnectionpool");
  config.Unlock();
}

CWebListener::~CWebListener()
{
  listener.Close();
  connectionpool->Stop();
  connectionpool->Wait();
  delete connectionpool;
}

// listener for incoming web connections, accepts the connection,
// and then puts the connection to connection pool
//
void * CWebListener::Run(void *arg)
{
CTCPConnection* connection;
  Log.Write("webonnectionlistener starting");
  if (listener.CreateListener(config.GetIntSetting("webserverport",8080),8)) {
    while (!Quitting()) {
      connection=new CTCPConnection;
      if (listener.Accept(*connection)) {
        Log.Write("Incoming web connection from %s",(char *)connection->remoteaddr);
        connectionpool->AddConnection(connection);
      }
    }
    Log.Write("webconnectionlistener quitting");
    listener.Close();
  }
  else
    Log.Write("CreateListener failed in webconnectionlistener: %s",strerror(errno));
  Log.Write("Web Connection listener stopped");
  return NULL;
}


