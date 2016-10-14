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
#include "connectionpool.hpp"

CConnectionPool::CConnectionPool(int size)
{
  connections=new CTCPConnection*[size];
  for (int i=0;i<size;i++)
    connections[i]=NULL;
  maxconnections=size;
  activeconnections=0;
  Log.Write("Connection pool for %d connections created",size);
}

CConnectionPool::~CConnectionPool()
{
  mutex.Lock();
  for (int i=0;i<maxconnections;i++) {
    if (connections[i]) {
      connections[i]->Close();
      delete connections[i];
    }
  }
  delete connections;
  Log.Write("Connection pool destroyed");
  mutex.Unlock();
}

bool CConnectionPool::AddConnection(CTCPConnection* c)
{
  mutex.Lock();
  if (activeconnections<maxconnections) {
    for (int i=0;i<maxconnections;i++) {
      if (!connections[i]) {
        activeconnections++;
        connections[i]=c;
        Log.Write("Connection to %s added to connection pool",(char *)c->remoteaddr);
        mutex.Unlock();
        return true;
      }
    }
    Log.Write("AddConnection confusion!");
  }
  else
    Log.Write("Connection pool full, cannot add another connection");
  c->Close();
  delete c;
  mutex.Unlock();
  return false;
}

void CConnectionPool::BroadCast(uchar *message,int l)
{
  mutex.Lock();
  for (int i=0;i<maxconnections;i++) {
    if (connections[i]) {
      if (connections[i]->Write((char *)message,l,MSG_DONTWAIT)!=l) {
        if (connections[i]->GetSd()!=-1) {
          Log.Write("Closing broken connection to %s",(char *)connections[i]->remoteaddr);
          connections[i]->Close();
        }
        delete connections[i];
        connections[i]=NULL;
        activeconnections--;
      }
    }
  }
  mutex.Unlock();
}

void CConnectionPool::BroadCast(char *message) 
{
  int l=strlen(message);
  BroadCast((uchar *)message,l);
}

void CConnectionPool::StatusPage(CTCPConnection& c)
{
  char*s="<table border=1>\n",buf[128];
  mutex.Lock();
  if (activeconnections) {
    c.Write(s,strlen(s));
    for (int i=0;i<maxconnections;i++) {
      if (connections[i]) {
        sprintf(buf,"<tr><td>%s</td></tr>\n",(char *)connections[i]->remoteaddr);
        c.Write(buf,strlen(buf));
      }
    }
    s="</table>\n";
    c.Write(s,strlen(s));
  }
  mutex.Unlock();
}

// TODO: polling is stupid, should do a proper select with timeout instead
void CConnectionPool::Poll()
{
  char buf[1024],*s;
  int l,i;
  mutex.Lock();
  for (i=0;i<maxconnections;i++) {
    if (connections[i]) {
      if (connections[i]->GetSd()==-1) {
        delete connections[i];
        connections[i]=NULL;
        activeconnections--;
      }
      else {
        if (connections[i]->DataReady()) {
          l=connections[i]->Read(buf,sizeof(buf)-1,MSG_DONTWAIT);
          if (l>0) {
            buf[l]='\0';
            s=buf+l-1;
            while (s>=buf && isspace(*s))
              *s--='\0';
            s=strchr(buf,'\n');
            if (s)
              *s='\0';
            s=strchr(buf,'\r');
            if (s)
              *s='\0';
            if (*buf)
              onClientData(*connections[i],buf,strlen(buf));
          }
        }
      }
    }
  }
  mutex.Unlock();
}
