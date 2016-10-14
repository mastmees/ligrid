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
#ifndef __connectionpool_hpp__
#define __connectionpool_hpp__

#include "ipnetwork.hpp"
#include "log.hpp"
#include "thread.hpp"

// generic pool of tcp connections, incoming data must be polled for
// the main usage is to maintain various client connections, and to
// broadcast messages to all of them
//
class CConnectionPool
{

private:

  CTCPConnection **connections;
  int maxconnections;
  int activeconnections;
  CMutex mutex;
  
public:

  // inherit & override to handle incoming data
  virtual void onClientData(CTCPConnection& connection,char *buf,int len) {}

  // connection pool can hold finite number of connections, defaults to 32
  // if connection pool is full, then any additional incoming connections
  // are just dropped
  //
  CConnectionPool(int size=32);
  virtual ~CConnectionPool();
  
  // add new connection to pool
  bool AddConnection(CTCPConnection* c);
  // send message of l bytes to all connections currently in the pool
  void BroadCast(uchar *message,int l);
  // send a zero-terminated string as message to all connections in the pool
  // note that 0 byte is not sent
  void BroadCast(char *message);
  // poll for any incoming data from active connections, calls onClientData()
  // to handle
  void Poll();
  // renders its own part of html statuspage
  void StatusPage(CTCPConnection& c);

};

#endif
