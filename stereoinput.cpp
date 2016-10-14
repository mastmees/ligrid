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

#include "stereoinput.hpp"

#ifdef __linux__

#include <sys/soundcard.h>
#include "ligrid.hpp"

// Audio on linux is really painful. The OSS interface below works
// for me, OSS is emulated by ALSA on my system
// TODO: allow more modern sound systems than OSS to be used natively

class CLinuxStereoInput : public CStereoInput
{
private:
    int ifd;
    int input;
public:

    bool Open(const char *device) {
        ifd=open(device,O_RDONLY,0);
        if (ifd==-1)
            Log.Write("Failed to open audio device %s (%s)",device,strerror(errno));
        else
            Log.Write("Audio device %s opened",device);
        return (ifd!=-1);
    }
    
    void Close()
    {
        if (ifd>=0)
          close(ifd);
        ifd=-1;
    }
    
    bool SetRecordingSource(RECORDINGSOURCE s)
    {
        input=SOUND_MIXER_LINE;
        switch (s) {
            case LINE:
                input=SOUND_MIXER_LINE;
                break;
            case LINE1:
                input=SOUND_MIXER_LINE1;
                break;
            case LINE2:
                input=SOUND_MIXER_LINE2;
                break;
            case LINE3:
                input=SOUND_MIXER_LINE3;
                break;
        }
        if (ioctl(ifd,MIXER_WRITE(SOUND_MIXER_RECSRC),&input)==-1) {
            Log.Write("Cannot set recording source (%s)",strerror(errno));
            return false;
        }
        int format=AFMT_S16_LE;
        if (ioctl(ifd,SNDCTL_DSP_SETFMT,&format)==-1) {
            Log.Write("Cannot set audio format (%s)",strerror(errno));
            return false;
        }
        if (format!=AFMT_S16_LE) {
            Log.Write("Audio device does not support 16-bit little-endian samples");
            return false;
        }
        int channels=2;
        // sound system may lie about channel being stereo, typically does
        // for mic inputs. Line inputs should be fine.
        if (ioctl(ifd,SNDCTL_DSP_CHANNELS,&channels)==-1) {
            Log.Write("Audio device does not support stereo (%s)",strerror(errno));
            return false;
        }
        if (channels!=2) {
          Log.Write("Audio device has %d channels, but we need 2",channels);
          return false;
        }
        Log.Write("Recording source set to %d",(int)s);
        return true;
    }
    
    bool SetRecordingVolume(int percent)
    {
        // not wanting to control overall gain and specific input gain
        // separately, I'm setting both to same level, doubling the overall
        // effect
        int x=(percent<<8)|percent;
        if (ioctl(ifd,MIXER_WRITE(input),&x)==-1) {
            Log.Write("Cannot set recording volume (%s)",strerror(errno));
            return false;
        }
        x=(percent<<8)|percent;
        if (ioctl(ifd,MIXER_WRITE(SOUND_MIXER_IGAIN),&x)==-1) {
            Log.Write("Cannot set input gain (%s)",strerror(errno));
            return false;
        }
        Log.Write("Recording volume set to %d%%",percent);
        return true;
    }

    // sound system may report that it supports basically any sampling
    // rate that you want, and then resample from physically available
    // rate. to make things worse, sometimes audio hardware only has
    // antialias filter for its highest sampling rate, even though
    // hardware allows lower sampling rates too. The only way around this
    // hardware problem is to sample at highest rate, then filter and down-
    // sample in software
        
    bool SetSampleRate(int samplespersecond)
    {
        int speed=samplespersecond;
        if (ioctl(ifd,SNDCTL_DSP_SPEED,&speed)==-1) {
            Log.Write("Cannot set sampling rate (%s)",strerror(errno));
            return false;
        }
        if (speed!=samplespersecond) {
            Log.Write("Audio device does not support %d Hz sampling rate\n",samplespersecond);
            return false;
        }
        Log.Write("Sampling rate set to %d Hz",samplespersecond);
        return true;
    }
    
    int Read(void *buf,int nbytes)
    {
        int i=read(ifd,buf,nbytes);
        if (i!=nbytes)
            Log.Write("Audio data read read error (%s)",strerror(errno));
        return i;
    }

};

CLinuxStereoInput _inputdevice;
#endif

#ifdef __APPLE__
class CAppleStereoInput : public CStereoInput
{
private:
public:

    bool Open(const char *device) {
        return false;
    }
    
    void Close()
    {
    }
    
    bool SetRecordingSource(RECORDINGSOURCE s)
    {
        return false;
    }
    
    bool SetRecordingVolume(int percent)
    {
        return false;
    }
        
    bool SetSampleRate(int samplespersecond)
    {
        return false;
    }
    
    int Read(void *buf,int nbytes)
    {
        return -1;
    }

};

CAppleStereoInput _inputdevice;
#endif

CStereoInput& StereoInputDevice = _inputdevice;
