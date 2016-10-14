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
#ifndef __connectionthread_h_included__
#define __connectionthread_h_included__
#include "../juce/juce.h"
#include "../ipnetwork.hpp"
#include "scopeComponent.h"
#include "powerscopeComponent.h"
#include "directionscopeComponent.h"
#include "radarComponent.h"

class ConnectionThread : public Thread
{
private:
	CTCPConnection *connection;
	// this isnt the best way to get the data around, connectionthread
	// just updates the scopes itself instead of distributing incoming
	// data to components
	scopeComponent* scope;
	powerscopeComponent* powerscope;
	directionscopeComponent* directionscope;
	radarComponent* radarscope;

public:

	ConnectionThread () :  Thread (T("ConnectionThead"))
	{
		//stations=NULL;
		connection=NULL;
		scope=NULL;
		powerscope=NULL;
		directionscope=NULL;
	}

	void SetScope(scopeComponent *s)
	{
		scope=s;
	}

	void SetPowerScope(powerscopeComponent *s)
	{
		powerscope=s;
	}

	void SetDirectionScope(directionscopeComponent *s)
	{
		directionscope=s;
	}
	
	void SetRadarScope(radarComponent *s)
	{
		radarscope=s;
	}
	
	void Start(CTCPConnection *c)
	{
		connection=c;
		startThread();
	}
	
	void Stop()
	{
		if (connection && connection->GetSd()!=-1) {
			connection->Write((void*)"#q\r",3,MSG_DONTWAIT);
			wait(100);
		}
		stopThread(2000);
	}
	
	~ConnectionThread()
	{
		Stop();
	}

	void SendStrikeData(char *s)
	{
		const MessageManagerLock mmLock;
		char *t=s;
		int samples[400],i;
		for (i=0;i<3 && t;i++) {
			t=strchr(t,',');
			if (t) t++;
		}
		double level=strtod(t,NULL);
		for (i=0;i<4 && t;i++) {
			t=strchr(t,',');
			if (t) t++;
		}
		if (t) {
			int nsamples=atoi(t);
			for (i=0;i<nsamples*2 && t;i++) {
				t=strchr(t,',');
				if (t) {
					t++;
					if (i<400) {
						samples[i]=atoi(t);
					}
				}
			}
			if (scope) {
				scope->SetData(nsamples>200?200:nsamples,samples);
			}
			if (powerscope) {
				powerscope->SetData(nsamples>200?200:nsamples,samples);
			}
			if (directionscope && t) {
				double a,b;
				t=strchr(t,',');
				if (t) {
					t++;
					a=strtod(t,NULL);
					t=strchr(t,',');
					if (t) {
						b=strtod(t+1,NULL);
						directionscope->SetData(level,nsamples>200?200:nsamples,samples,a,b);
					}
				}
			}
		}
	}
	
	void run() {
		connection->Write((void *)"#R\r",3,MSG_DONTWAIT);
		while (!threadShouldExit() && connection && connection->GetSd()!=-1) {
			if (connection->DataReady()) {
				char s[8192];
				if (connection->ReadLine(s,sizeof(s))>=0) {
					if (s[0]=='#') {
						switch (toupper(s[1])) {
							case 'D':
								SendStrikeData(s);
								break;
							case 'R':
								radarscope->AddStation(s);
								break;
								
						}
					}
				}
				else {
					connection->Close();
					radarscope->ClearStations();
					return;
				}
			}
			else {
				wait(100);
			}
		}
		radarscope->ClearStations();
	}
	
};

extern ConnectionThread Dataconnection;
#endif
