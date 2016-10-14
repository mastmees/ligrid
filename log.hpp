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
#ifndef __log_hpp__
#define __log_hpp__

#include <stdio.h>
#include <stdarg.h>

// simpleminded log writer, adds timestamp in front of printf() style
// formatted log entry, and adds newline at the end
//
// by default logs to stdout, call SetOutput() to redirect to file
//
// TODO: send log output to syslog
class CLog
{
FILE *outfile;

public:
  CLog()
  {
    outfile=stdout;
  }
  
  ~CLog()
  {
    if (outfile && outfile!=stdout)
      fclose(outfile);
  }
  
  void SetOutput(const char *filename)
  {
    if (!filename || *filename=='\0')
      outfile=stdout;
    outfile=fopen(filename,"at");
    if (!outfile)
      outfile=stdout;
    Write("Logging to %s",(outfile!=stdout)?filename:"stdout");
  }
  
  void Write(const char *format,...)
  {
    char buf[2048];
    time_t t;
    struct tm *tm;
    t=time(NULL);
    tm=localtime(&t);
    sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",
      tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
      tm->tm_hour,tm->tm_min,tm->tm_sec);
    va_list ap;
    va_start(ap,format);
    vsnprintf(buf+20,sizeof(buf)-20,format,ap);
    fprintf(outfile,"%s\n",buf);
    fflush(outfile);
	va_end(ap);
  }
};

extern CLog Log;

#endif
