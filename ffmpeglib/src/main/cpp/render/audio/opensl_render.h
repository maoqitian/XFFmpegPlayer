//
// Created by maoqitian on 2020/12/17 0017.
// Description: OpenSL ES 音频播放渲染
// Android 上播放音频，通常使用的 AudioTrack ，但是在 NDK 层，没有提供直接的类，需要通过 NDK 调用 Java 层的方式，回调实现播放
// NDK 层，提供另一种播放音频的方法：OpenSL ES
//

#ifndef FFMPEGPLAYER_OPENSL_RENDER_H
#define FFMPEGPLAYER_OPENSL_RENDER_H

#include <cstdint>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <string>
#include <thread>

#include "audio_render.h"

extern "C" {
#include <libavutil/mem.h>
}

#define MAX_QUEUE_BUFFER_SIZE 3


static const char *TAG = "OpenSLRender";

class OpenSLRender : public AudioRender {
public:
    OpenSLRender(){}
    virtual ~OpenSLRender(){}
    virtual void Init();
    virtual void ClearAudioCache();
    virtual void RenderAudioFrame(uint8_t *pData, int dataSize);
    virtual void UnInit();

private:
    int CreateEngine();
    int CreateOutputMixer();
    int CreateAudioPlayer();
    int GetAudioFrameQueueSize();
    void StartRender();
    void HandleAudioFrameQueue();
    static void CreateSLWaitingThread(OpenSLRender *openSlRender);
    static void AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    SLObjectItf m_EngineObj = nullptr;
    SLEngineItf m_EngineEngine = nullptr;
    SLObjectItf m_OutputMixObj = nullptr;
    SLObjectItf m_AudioPlayerObj = nullptr;
    SLPlayItf m_AudioPlayerPlay = nullptr;
    SLVolumeItf m_AudioPlayerVolume = nullptr;
    SLAndroidSimpleBufferQueueItf m_BufferQueue;

    std::queue<AudioFrame *> m_AudioFrameQueue;

    std::thread *m_thread = nullptr;
    std::mutex   m_Mutex;
    std::condition_variable m_Cond;
    volatile bool m_Exit = false;
};


#endif //FFMPEGPLAYER_OPENSL_RENDER_H
