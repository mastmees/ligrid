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

const char *devnames[] = {
"SOUND_MIXER_VOLUME",
"SOUND_MIXER_BASS",
"SOUND_MIXER_TREBLE",
"SOUND_MIXER_SYNTH",
"SOUND_MIXER_PCM",
"SOUND_MIXER_SPEAKER",
"SOUND_MIXER_LINE",
"SOUND_MIXER_MIC",
"SOUND_MIXER_CD",
"SOUND_MIXER_IMIX",
"SOUND_MIXER_ALTPCM",
"SOUND_MIXER_RECLEV",
"SOUND_MIXER_IGAIN",
"SOUND_MIXER_OGAIN",
"SOUND_MIXER_LINE1",
"SOUND_MIXER_LINE2",
"SOUND_MIXER_LINE3",
"SOUND_MIXER_DIGITAL1",
"SOUND_MIXER_DIGITAL2",
"SOUND_MIXER_DIGITAL3",
"SOUND_MIXER_PHONEIN",
"SOUND_MIXER_PHONEOUT",
"SOUND_MIXER_VIDEO",
"SOUND_MIXER_RADIO",
"SOUND_MIXER_MONITOR"
};
const char *ossnames[]=SOUND_DEVICE_NAMES;


void showdevnames(int i)
{
  for (unsigned int j=0;j<COUNTOF(devnames);j++) {
    if (i&(1<<j)) {
      printf(" %s (%s)\n",ossnames[j],devnames[j]);
    }
  }  
}

int main(int arc,char *argv[])
{
int fd=open("/dev/mixer",O_RDONLY,0);
int i;
  if (fd==-1)
    perror("open");
  else {
    if (ioctl(fd,SOUND_MIXER_READ_DEVMASK,&i)==-1)
      perror("READ_DEVMASK");
    else {
      printf("Mixer supported devices:\n");
      showdevnames(i);
    }
    if (ioctl(fd,SOUND_MIXER_READ_RECSRC,&i)==-1)
      perror("READ_RECSRC");
    else {
      printf("Recording sources:\n");
      showdevnames(i);
    }
    if (ioctl(fd,SOUND_MIXER_READ_RECMASK,&i)==-1)
      perror("READ_RECMASK");
    else {
      printf("Recording devices:\n");
      showdevnames(i);
    }
    // beware, sound system may report pyhiscally mono devices as stereo
    // ones, and copy same signal to both channels
    if (ioctl(fd,SOUND_MIXER_READ_STEREODEVS,&i)==-1)
      perror("READ_STEREODEVS");
    else {
      printf("Stereo channels:\n");
      showdevnames(i);
    }
    close(fd);  
  }
}
