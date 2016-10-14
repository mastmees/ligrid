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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>         
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../types.hpp"
#include <time.h>
#include "../filter.hpp"

#define RECORDING_DEVICE 	"/dev/dsp"  // default recording device
#define BLOCK_SIZE 			2048		// bytes per sampled block

class CAudio {

int ifd;					// recording device file descriptor

public:
  
  CAudio()
  {
    ifd=-1;
  }
  
  ~CAudio()
  {
    Close();
  }
  
  bool Open(int volume=50,const char *device=RECORDING_DEVICE,int samplerate=22050,int input=SOUND_MIXER_LINE)
  {
    int i,x;
    Close();
    ifd=open(device,O_RDONLY,0);
    if (ifd==-1)
      perror("CAudio::Open");
    if (ifd!=-1) {
      i=input;;
      if (ioctl(ifd,MIXER_WRITE(SOUND_MIXER_RECSRC),&i)==-1)
        perror("Cannot set recording source");
      x=(volume<<8)|volume;
      ioctl(ifd,MIXER_WRITE(input),&x);
      x=(volume<<8)|volume;
      ioctl(ifd,MIXER_WRITE(SOUND_MIXER_IGAIN),&x);
      int format=AFMT_S16_LE;
      if (ioctl(ifd,SNDCTL_DSP_SETFMT,&format)==-1) {
        perror("Cannot set audio format");
        return false;
      }
      if (format!=AFMT_S16_LE) {
        printf("%s does not support 16-bit little-endian samples\n",device);
        return false;
      }
      int channels=2;
      if (ioctl(ifd,SNDCTL_DSP_CHANNELS,&channels)==-1) {
        perror("Cannot switch to stereo");
        return false;
      }
      if (channels!=2) {
        printf("%s does not support stereo sampling\n",device);
        return false;
      }
      int speed=samplerate;
      if (ioctl(ifd,SNDCTL_DSP_SPEED,&speed)==-1) {
        perror("Cannot set sampling rate");
        return false;
      }
      if (speed!=samplerate) {
        printf("%s does not support %d Hz sampling rate\n",device,samplerate);
        return false;
      }
      return true;
    }
    return false;
  }
  
  void Close() 
  {
    if (ifd>=0) {
      int i;
      ioctl(ifd,SNDCTL_DSP_RESET,&i);
      close(ifd);
    }
    ifd=-1;
  }
  
  int Read(char *buf,int len)
  {
    int i;
    i=read(ifd,buf,len);
    if (i!=len)
      perror("Audio data read");
    return i;
  }

};

double fc[]= {
0.100,
-0.400,
0.600,
-0.400,
0.100,
};

int main(int argc,char *argv[])
{
CAudio audio;
unsigned int n,i;
char buf[8192];
short *pcm;
sprintf(buf,"%08x.dat",(unsigned int)time(NULL));
FILE *fp=fopen(buf,"wb");
FIR left(COUNTOF(fc),fc);
FIR right(COUNTOF(fc),fc);
  if (fp) {
    printf("Writing to %s\n",buf);
    if (audio.Open(20)) {
      for (int b=0;b<220;b++) {
        if ((n=audio.Read(buf,sizeof(buf)))!=sizeof(buf))
          break;
        else {

          for (i=0;i<sizeof(buf);i+=4) {
             pcm=(short*)(buf+i);
             pcm[0]=(short)(left.Filter(pcm[0])+0.5);
             pcm[1]=(short)(right.Filter(pcm[1])+0.5);
          }
          if (b>19)
            fwrite(buf,n,1,fp);
        }
      }
      audio.Close();
    }
    fclose(fp);
  }
}
