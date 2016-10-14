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
#ifndef __ligrid_hpp__
#define __ligrid_hpp__

#define VERSIONSTRING 		"V0.0.1(Linux)"

#define BLOCK_SIZE 			2048		// bytes per sampled block
#define STRIKE_WINDOW_SIZE	200			// samples in window
#define STRIKE_LEAD_SAMPLES	30			// samples before trigger
#define STRIKE_GUARD_SAMPLES 170		// guard time samples
                                        // no new strikes will be registered
                                        // in guard time window from
                                        // strike detection point
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>         
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>

#include "ipnetwork.hpp"
#include "stereoinput.hpp"
#include "config.hpp"
#include "thread.hpp"
#include "averagepower.hpp"
#include "pointerqueue.hpp"
#include "filter.hpp"
#include "types.hpp"
#include "strikes.hpp"
#include "sampler.hpp"
#include "connectionpool.hpp"
#include "uiconnection.hpp"
#include "clientconnection.hpp"
#include "signalscanner.hpp"
#include "sampler.hpp"
#include "log.hpp"
#include "stationcache.hpp"
#include "webconnection.hpp"

extern char *GetTime(char *s);

extern CConfig config;
extern CLog Log;
extern CStationCache *stationcache;
extern CListener *connectionlistener;
extern CUIListener *uiconnectionlistener;
extern CWebListener *weblistener;

#endif
