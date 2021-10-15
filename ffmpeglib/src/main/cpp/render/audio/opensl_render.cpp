//
// Created by maoqitian on 2020/12/17 0017.
// Description: OpenSL ES 音频播放渲染器
//

#include "opensl_render.h"
#include "AudioGLRender.h"
#include <thread>
#include <unistd.h>
#include <utils/logger.h>

//OpenSLES 渲染器初始化
void OpenSLRender::Init() {

    LOGCATE("OpenSLRender::Init");

    int result = -1;
    do {
        //创建并初始化引擎对象
        result = CreateEngine();
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::Init CreateEngine fail. result=%d", result);
            break;
        }
        //创建并初始化混音器
        result = CreateOutputMixer();
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::Init CreateOutputMixer fail. result=%d", result);
            break;
        }
        //创建并初始化播放器
        result = CreateAudioPlayer();
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::Init CreateAudioPlayer fail. result=%d", result);
            break;
        }
        //新线程 启动播放
        m_thread = new std::thread(CreateSLWaitingThread, this);

    } while (false);

    if(result != SL_RESULT_SUCCESS) {
        LOGCATE("OpenSLRender::Init fail. result=%d", result);
        UnInit();
    }

}

void OpenSLRender::ClearAudioCache() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    for (int i = 0; i < m_AudioFrameQueue.size(); ++i) {
        AudioFrame *audioFrame = m_AudioFrameQueue.front();
        m_AudioFrameQueue.pop();
        delete audioFrame;
    }
}

void OpenSLRender::RenderAudioFrame(uint8_t *pData, int dataSize) {
    LOGCATE("OpenSLRender::RenderAudioFrame pData=%p, dataSize=%d", pData, dataSize);
    if(m_AudioPlayerPlay) {
        if (pData != nullptr && dataSize > 0) {

            //temp resolution, when queue size is too big.
            while(GetAudioFrameQueueSize() >= MAX_QUEUE_BUFFER_SIZE && !m_Exit)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            std::unique_lock<std::mutex> lock(m_Mutex);
            AudioFrame *audioFrame = new AudioFrame(pData, dataSize);
            m_AudioFrameQueue.push(audioFrame);
            m_Cond.notify_all();
            lock.unlock();
        }
    }
}

void OpenSLRender::UnInit() {
    LOGCATE("OpenSLRender::UnInit");

    if (m_AudioPlayerPlay) {
        (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_STOPPED);
        m_AudioPlayerPlay = nullptr;
    }

    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Exit = true;
    m_Cond.notify_all();
    lock.unlock();

    if (m_AudioPlayerObj) {
        (*m_AudioPlayerObj)->Destroy(m_AudioPlayerObj);
        m_AudioPlayerObj = nullptr;
        m_BufferQueue = nullptr;
    }

    if (m_OutputMixObj) {
        (*m_OutputMixObj)->Destroy(m_OutputMixObj);
        m_OutputMixObj = nullptr;
    }

    if (m_EngineObj) {
        (*m_EngineObj)->Destroy(m_EngineObj);
        m_EngineObj = nullptr;
        m_EngineEngine = nullptr;
    }

    lock.lock();
    for (int i = 0; i < m_AudioFrameQueue.size(); ++i) {
        AudioFrame *audioFrame = m_AudioFrameQueue.front();
        m_AudioFrameQueue.pop();
        delete audioFrame;
    }
    lock.unlock();

    if(m_thread != nullptr)
    {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }

    //AudioGLRender::ReleaseInstance();
}
//Audio 引擎对象和接口
int OpenSLRender::CreateEngine() {
    SLresult result = SL_RESULT_SUCCESS;
    do {
        // 创建引擎对象
        result = slCreateEngine(&m_EngineObj, 0, nullptr, 0, nullptr, nullptr);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateEngine slCreateEngine fail. result=%d", result);
            break;
        }
        //// 实例化
        result = (*m_EngineObj)->Realize(m_EngineObj, SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateEngine Realize fail. result=%d", result);
            break;
        }
        //// 获取引擎对象接口
        result = (*m_EngineObj)->GetInterface(m_EngineObj, SL_IID_ENGINE, &m_EngineEngine);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateEngine GetInterface fail. result=%d", result);
            break;
        }

    } while (false);
    return result;
}

int OpenSLRender::CreateOutputMixer() {
    SLresult result = SL_RESULT_SUCCESS;
    do {
        const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

        result = (*m_EngineEngine)->CreateOutputMix(m_EngineEngine, &m_OutputMixObj, 1, mids, mreq);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateOutputMixer CreateOutputMix fail. result=%d", result);
            break;
        }

        result = (*m_OutputMixObj)->Realize(m_OutputMixObj, SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateOutputMixer CreateOutputMix fail. result=%d", result);
            break;
        }

    } while (false);

    return result;
}

int OpenSLRender::CreateAudioPlayer() {
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    //数据格式的定义
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//format type
            (SLuint32)2,//channel count
            SL_SAMPLINGRATE_44_1,//44100hz
            SL_PCMSAMPLEFORMAT_FIXED_16,// bits per sample
            SL_PCMSAMPLEFORMAT_FIXED_16,// container size
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,// channel mask
            SL_BYTEORDER_LITTLEENDIAN // endianness
    };
    //表示音频数据来源的信息
    SLDataSource slDataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, m_OutputMixObj};
    //表示音频数据输出信息
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLresult result;

    do {
        // 创建 audio player 对象
        result = (*m_EngineEngine)->CreateAudioPlayer(m_EngineEngine, &m_AudioPlayerObj, &slDataSource, &slDataSink, 3, ids, req);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer CreateAudioPlayer fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->Realize(m_AudioPlayerObj, SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer Realize fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_PLAY, &m_AudioPlayerPlay);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_BUFFERQUEUE, &m_BufferQueue);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

        result = (*m_BufferQueue)->RegisterCallback(m_BufferQueue, AudioPlayerCallback, this);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer RegisterCallback fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_VOLUME, &m_AudioPlayerVolume);
        if(result != SL_RESULT_SUCCESS)
        {
            LOGCATE("OpenSLRender::CreateAudioPlayer GetInterface fail. result=%d", result);
            break;
        }

    } while (false);

    return result;
}

int OpenSLRender::GetAudioFrameQueueSize() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_AudioFrameQueue.size();
}

void OpenSLRender::StartRender() {
    while (GetAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !m_Exit) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Cond.wait_for(lock, std::chrono::milliseconds(10));
        //m_Cond.wait(lock);
        lock.unlock();
    }
    //设置播放状态
    (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_PLAYING);
    //激活回调接口
    AudioPlayerCallback(m_BufferQueue, this);

}

void OpenSLRender::HandleAudioFrameQueue() {
    LOGCATE("OpenSLRender::HandleAudioFrameQueue QueueSize=%lu", m_AudioFrameQueue.size());
    if (m_AudioPlayerPlay == nullptr) return;

    while (GetAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !m_Exit) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Cond.wait_for(lock, std::chrono::milliseconds(10));
    }

    std::unique_lock<std::mutex> lock(m_Mutex);
    //播放存放在音频帧队列中的数据
    AudioFrame *audioFrame = m_AudioFrameQueue.front();
    if (nullptr != audioFrame && m_AudioPlayerPlay) {
        SLresult result = (*m_BufferQueue)->Enqueue(m_BufferQueue, audioFrame->data, (SLuint32) audioFrame->dataSize);
        if (result == SL_RESULT_SUCCESS) {
            AudioGLRender::GetInstance()->UpdateAudioFrame(audioFrame);
            m_AudioFrameQueue.pop();
            delete audioFrame;
        }

    }
    lock.unlock();
}

void OpenSLRender::CreateSLWaitingThread(OpenSLRender *openSlRender) {
    openSlRender->StartRender();
}
//播放器的 callback
void OpenSLRender::AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    OpenSLRender *openSlRender = static_cast<OpenSLRender *>(context);
    openSlRender->HandleAudioFrameQueue();
}
