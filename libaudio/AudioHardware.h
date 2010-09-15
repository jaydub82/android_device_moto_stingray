/*
** Copyright 2008-2010, The Android Open-Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_HARDWARE_H
#define ANDROID_AUDIO_HARDWARE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>
#include <utils/SortedVector.h>

#include <hardware_legacy/AudioHardwareBase.h>

extern "C" {
#include <linux/msm_audio.h>
#include "cto_audio_mm.h"
}

namespace android {

#include <linux/cpcap_audio.h>
#include <linux/tegra_audio.h>

#define AUDIO_HW_NUM_OUT_BUF 4
#define AUDIO_HW_OUT_LATENCY_MS 0

#define AUDIO_HW_IN_SAMPLERATE 11025                  // Default audio input sample rate
#define AUDIO_HW_IN_CHANNELS (AudioSystem::CHANNEL_IN_MONO) // Default audio input channel mask
#define AUDIO_HW_IN_BUFFERSIZE (4096)               // Default audio input buffer size
#define AUDIO_HW_IN_FORMAT (AudioSystem::PCM_16_BIT)  // Default audio input sample format

class AudioHardware : public  AudioHardwareBase
{
    class AudioStreamOutTegra;
    class AudioStreamInTegra;

public:
                        AudioHardware();
    virtual             ~AudioHardware();
    virtual status_t    initCheck();

    virtual status_t    setVoiceVolume(float volume);
    virtual status_t    setMasterVolume(float volume);

    virtual status_t    setMode(int mode);

    // mic mute
    virtual status_t    setMicMute(bool state);
    virtual status_t    getMicMute(bool* state);

    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);

    // create I/O streams
    virtual AudioStreamOut* openOutputStream(
                                uint32_t devices,
                                int *format=0,
                                uint32_t *channels=0,
                                uint32_t *sampleRate=0,
                                status_t *status=0);

    virtual AudioStreamIn* openInputStream(

                                uint32_t devices,
                                int *format,
                                uint32_t *channels,
                                uint32_t *sampleRate,
                                status_t *status,
                                AudioSystem::audio_in_acoustics acoustics);

    virtual    void        closeOutputStream(AudioStreamOut* out);
    virtual    void        closeInputStream(AudioStreamIn* in);

    virtual    size_t      getInputBufferSize(uint32_t sampleRate, int format, int channelCount);

protected:
    virtual status_t    dump(int fd, const Vector<String16>& args);

private:

    status_t    setMicMute_nosync(bool state);
    status_t    checkMicMute();
    status_t    dumpInternals(int fd, const Vector<String16>& args);
    uint32_t    getInputSampleRate(uint32_t sampleRate);
    status_t    doStandby(bool output, bool enable);
    status_t    doRouting();

    AudioStreamInTegra*   getActiveInput_l();
    uint32_t    convOutDevToCTO(uint32_t outDev);
    uint32_t    convRateToCto(uint32_t rate);
    void        setCtoAudioRate(int rate);
    void        setCtoAudioDev(uint32_t outDev, uint32_t inDev);
    void        configCtoAudio();

    class AudioStreamOutTegra : public AudioStreamOut {
    public:
                            AudioStreamOutTegra();
        virtual             ~AudioStreamOutTegra();
                status_t    set(AudioHardware* mHardware,
                                uint32_t devices,
                                int *pFormat,
                                uint32_t *pChannels,
                                uint32_t *pRate);
        virtual uint32_t    sampleRate() const { return 44100; }
        // must be 32-bit aligned - driver only seems to like 4800
        virtual size_t      bufferSize() const { return 4096; }
        virtual uint32_t    channels() const { return AudioSystem::CHANNEL_OUT_STEREO; }
        virtual int         format() const { return AudioSystem::PCM_16_BIT; }
        virtual uint32_t    bytesPerSample() const { int ret = 1;
                                                     if (format()==AudioSystem::PCM_16_BIT) ret*=2;
                                                     if (channels()==AudioSystem::CHANNEL_OUT_STEREO) ret*=2;
                                                     return ret; }
        virtual uint32_t    latency() const { return (1000*AUDIO_HW_NUM_OUT_BUF*(bufferSize()/frameSize()))/sampleRate()+AUDIO_HW_OUT_LATENCY_MS; }
        virtual status_t    setVolume(float left, float right) { return INVALID_OPERATION; }
        virtual ssize_t     write(const void* buffer, size_t bytes);
        virtual status_t    standby();
        virtual status_t    online();
        virtual status_t    dump(int fd, const Vector<String16>& args);
                bool        getStandby();
        virtual status_t    setParameters(const String8& keyValuePairs);
        virtual String8     getParameters(const String8& keys);
                uint32_t    devices() { return mDevices; }
        virtual status_t    getRenderPosition(uint32_t *dspFrames);
    private:
                AudioHardware* mHardware;
                int         mFd;
                int         mFdCtl;
                int         mStartCount;
                int         mRetryCount;
                uint32_t    mDevices;
    };

    class AudioStreamInTegra : public AudioStreamIn {
    public:
        enum input_state {
            AUDIO_INPUT_CLOSED,
            AUDIO_INPUT_OPENED,
            AUDIO_INPUT_STARTED
        };

                            AudioStreamInTegra();
        virtual             ~AudioStreamInTegra();
                status_t    set(AudioHardware* mHardware,
                                uint32_t devices,
                                int *pFormat,
                                uint32_t *pChannels,
                                uint32_t *pRate,
                                AudioSystem::audio_in_acoustics acoustics);
        virtual size_t      bufferSize() const { return mBufferSize; }
        virtual uint32_t    channels() const { return mChannels; }
        virtual int         format() const { return mFormat; }
        virtual uint32_t    sampleRate() const { return mSampleRate; }
        virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
        virtual ssize_t     read(void* buffer, ssize_t bytes);
        virtual status_t    dump(int fd, const Vector<String16>& args);
        virtual status_t    standby();
        virtual status_t    online();
                bool        getStandby();
        virtual status_t    setParameters(const String8& keyValuePairs);
        virtual String8     getParameters(const String8& keys);
        virtual unsigned int  getInputFramesLost() const { return 0; }
                uint32_t    devices() { return mDevices; }
                int         state() const { return mState; }

    private:
                AudioHardware* mHardware;
                int         mFd;
                int         mFdCtl;
                int         mState;
                int         mRetryCount;
                int         mFormat;
                uint32_t    mChannels;
                uint32_t    mSampleRate;
                size_t      mBufferSize;
                AudioSystem::audio_in_acoustics mAcoustics;
                uint32_t    mDevices;
    };

            static const uint32_t inputSamplingRates[];
            bool        mInit;
            bool        mMicMute;
            bool        mBluetoothNrec;
            uint32_t    mBluetoothId;
            AudioStreamOutTegra*  mOutput;
            SortedVector <AudioStreamInTegra*>   mInputs;

            struct cpcap_audio_stream mCurOutDevice;
            struct cpcap_audio_stream mCurInDevice;
        // CTO Audio Processing storage buffers
            int16_t     mPcmLoggingBuf[((CTO_AUDIO_MM_DATALOGGING_BUFFER_BLOCK_BYTESIZE)/2)];
            uint32_t    mNoiseEst[((CTO_AUDIO_MM_NOISE_EST_BLOCK_BYTESIZE)/4)];
            uint16_t    mRuntimeParam[((CTO_AUDIO_MM_RUNTIME_PARAM_BYTESIZE)/2)];
            uint16_t    mStaticMem[((CTO_AUDIO_MM_STATICMEM_BLOCK_BYTESIZE)/2)];
            uint16_t    mScratchMem[((CTO_AUDIO_MM_SCRATCHMEM_BLOCK_BYTESIZE)/2)];
            CTO_AUDIO_MM_ENV_VAR mAudioMmEnvVar;

     friend class AudioStreamInTegra;
            Mutex       mLock;
            Mutex       mCtoLock;

            int mCpcapCtlFd;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_AUDIO_HARDWARE_MSM72XX_H
