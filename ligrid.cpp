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

// these are not affected by restart
//
CConfig config;
CLog Log;
FILE *DataFile;
CSampler sampler;
CStationCache *stationcache;

// but these are recreated
//
CListener *connectionlistener;
CUIListener *uiconnectionlistener;
CWebListener *weblistener;

char *GetTime(char *s)
{
time_t t;
struct tm *tm;
  t=time(NULL);
  tm=localtime(&t);
  sprintf(s,"%04d/%02d/%02d %02d:%02d:%02d",
    tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
    tm->tm_hour,tm->tm_min,tm->tm_sec);
  return s;
}

// this method is invoked whenever a lightning strike is
// detected and its sample window filled
//
void CStrikes::StrikeDetected(CStrikes::STRIKE *s)
{
char d[128];
char buf[8192],sbuf[8192],*t;
int i,left,right;
  GetTime(d);
  // convert level back to value
  double level=pow(10,(s->level/20.0))*32768.0;
  level=level*config.GetDoubleSetting("thresholdratio",1.0);
  t=sbuf;
  double sx=0.0,sx2=0.0,sy=0.0,sxy=0.0,mag;
  int n=0;
  for (i=0;i<s->nbytes;i+=4) {
    left=(short)(s->samples[i]|(s->samples[i+1]<<8));
    right=(short)(s->samples[i+2]|(s->samples[i+3]<<8));
    sprintf(t,",%d,%d",left,right);
    // for bearing finding, use up to 100 samples where power is above threshold
    // this speculatively accumulates values for later calculations
    // to solve the linear least squares equation
    // (see http://mathworld.wolfram.com/LeastSquaresFitting.html)
    mag=sqrt((double)left*(double)left+(double)right*(double)right);
    if (mag>level && n<100) {
      sx2=sx2+(double)left*(double)left;
      sx=sx+(double)left;
      sy=sy+(double)right;
      sxy=sxy+(double)left*(double)right;
      n++;
    }
    t=strchr(t,'\0');
  }
  // if more than one point, and at least one is at nonzero,
  // solve the equation
  if (n>1 && sx) {
    double a,b,r,maxr=0.0,maxe=0.0;
    a=(sy*sx-n*sxy)/(sx*sx-n*sx2);
    b=(sx*sxy-sy*sx2)/(sx*sx-n*sx2);
    // with a and b calculated, check the quality of fit now
    if (a) {
      double a1=-1/a;
      for (i=0;i<s->nbytes;i+=4) {
        left=(short)(s->samples[i]|(s->samples[i+1]<<8));
        right=(short)(s->samples[i+2]|(s->samples[i+3]<<8));
        r=left*left+right*right;
        if (r>maxr) maxr=r;
        double b1=(double)right-a1*(double)left;
        double x=(b1-b)/(a-a1);
        double y=a*x+b;
        x=x-(double)left;
        y=y-(double)right;
        double e=x*x+y*y;
        if (e>maxe) maxe=e;
      }
      maxe=100.0*sqrt(maxe)/sqrt(maxr);
    }
    // if sufficiently good fit then broadcast to both pools
    if (maxe<config.GetDoubleSetting("goodfitquality",10.0)) {
      double bearing;
      bearing=(180.0/M_PI)*atan((a*100.0+b)/100.0);
      bearing=bearing+(180.0/M_PI)*atan((a*-100.0+b)/-100.0);
      bearing=bearing/2.0;
      bearing=90-bearing;	// reorient origin
      bearing=fmod(bearing+config.GetDoubleSetting("bearingadjustment",0.0)+360.0,180.0);
      sprintf(t,",%f,%f",a,b);
      strcat(t,"\r");
      sprintf(buf,"#D%s,%.2f,%10d,%f,%010u,%s,%s,%d%s",
        d,
        bearing,
        (int)s->clock,
        20.0*log10(level/32768.0),
        0,
        config.GetStringSetting("latitude",""),
        config.GetStringSetting("longitude",""),
        s->nbytes/4,
        sbuf
      );    
      if (DataFile) {
        fprintf(DataFile,"%s\n",buf);
        fflush(DataFile);
      }
      uiconnectionlistener->connectionpool->BroadCast(buf);
      sprintf(buf,"%s,%.2f,%010d,%f,%010u,%s,%s,%s\r",
        d,
        bearing,
        (int)s->clock,
        s->level,
        0,
        config.GetStringSetting("latitude",""),
        config.GetStringSetting("longitude",""),
        config.GetStringSetting("stationname",""));
      //2009/02/20 01:05:40,232.28,0997168757,-083.6,000000134,52.25056,06.80556,Hengelo the Netherlands
      connectionlistener->connectionpool->BroadCast(buf);
    }
  }
}

int killed,restarting;

void breakhandler(int i) { killed++; }
void huphandler(int i) { restarting++; }
               
int main(int argc,char *argv[])
{
  signal(SIGINT,breakhandler);
  signal(SIGHUP,huphandler);
  signal(SIGTERM,breakhandler);
  signal(SIGPIPE,SIG_IGN);
  config.Load("ligrid.conf");
  Log.SetOutput(config.GetStringSetting("logfile",""));
  const char *df=config.GetStringSetting("datafile","");
  if (*df) {
    DataFile=fopen(df,"at");
  }
  stationcache=new CStationCache;
  if (config.GetBoolSetting("daemonize",false)==true) {
    int pid=fork();
    if (pid<0) {
      Log.Write("Could not daemonize, fork failed (%s)",strerror(errno));
      return 1;
    }
    if (pid>0) {
      return 0; // leave the parent
    }
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }
  stationcache->Start("stationcache");
  // stays in this loop for restarts
  while (1) {
    restarting=0;
    connectionlistener=new CListener;
    uiconnectionlistener=new CUIListener;
    weblistener=new CWebListener;
    weblistener->Start("weblistener");
    connectionlistener->Start("connectionlistener");
    uiconnectionlistener->Start("uiconnectionlistener");
    signalscanner.Start("signalscanner");

    while (!killed && !restarting) {
      connectionlistener->connectionpool->Poll();
      uiconnectionlistener->connectionpool->Poll();
      // TODO: should share the station cache with connected nodes, but dont know the rules
      usleep(10000);
    }
    Log.Write("Stopping signalscanner");
    signalscanner.Stop();
    Log.Write("Stopping connectionlistener");
    connectionlistener->Close();
    connectionlistener->Stop();
    Log.Write("Stopping uiconnectionlistener");
    uiconnectionlistener->Close();
    uiconnectionlistener->Stop();
    Log.Write("Stopping weblistener");
    weblistener->Close();
    weblistener->Stop();
    sleep(1);
    signalscanner.Wait();
    connectionlistener->Wait();
    uiconnectionlistener->Wait();
    weblistener->Wait();
    delete connectionlistener;
    delete uiconnectionlistener;
    delete weblistener;
    if (killed)
      break;
    Log.Write("Restarting");
  }
  Log.Write("Stopping stationcache");
  stationcache->Stop();
  stationcache->Wait();
  delete stationcache;
  if (DataFile)
    fclose(DataFile);
  Log.Write("Shutting down");
  return 0;
}
