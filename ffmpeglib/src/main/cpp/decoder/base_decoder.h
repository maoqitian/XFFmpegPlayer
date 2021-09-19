//
// Created by maoqitian on 2020/12/15 0015.
// 解码器基础类

#ifndef FFMPEGPLAYER_BASE_DECODER_H
#define FFMPEGPLAYER_BASE_DECODER_H

#include <jni.h>
#include <string>
#include <thread>
#include "i_decoder.h"
#include "decode_state.h"
#include "utils/FFLog.h"
#include "i_decode_state_cb.h"
#include <utils/logger.h>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavcodec/jni.h>
}


#define MAX_PATH   2048
//音视频 时间延迟 阈值
#define DELAY_THRESHOLD 100 //100ms

using namespace std;

enum DecoderState {
    STATE_UNKNOWN,
    STATE_DECODING,
    STATE_PAUSE,
    STATE_STOP
};

enum DecoderMsg {
    MSG_DECODER_INIT_ERROR,
    MSG_DECODER_READY,
    MSG_DECODER_DONE,
    MSG_REQUEST_RENDER,
    MSG_DECODING_TIME
};

class BaseDecoder : public IDecoder{
    //-------------定义解码相关------------------------------
private:
    int InitFFDecoder();
    void UnInitDecoder();
    //启动解码线程
    void StartDecodingThread();
    //音视频解码循环
    void DecodingLoop();
    //更新显示时间戳
    void UpdateTimeStamp();
    //音视频同步
    long AVSync();
    //解码一个packet编码数据
    int DecodeOnePacket();
    //线程函数
    static void DoAVDecoding(BaseDecoder *decoder);

    //封装格式上下文
    AVFormatContext *m_AVFormatContext = nullptr;
    //解码器上下文
    AVCodecContext  *m_AVCodecContext = nullptr;
    //解码器
    AVCodec         *m_AVCodec = nullptr;
    //编码的数据包
    AVPacket        *m_Packet = nullptr;
    //解码的帧
    AVFrame         *m_Frame = nullptr;
    //数据流的类型
    AVMediaType      m_MediaType = AVMEDIA_TYPE_UNKNOWN;
    //文件地址
    char       m_Url[MAX_PATH] = {0};
    //当前播放时间
    long             m_CurTimeStamp = 0;
    //播放的起始时间
    long             m_StartTimeStamp = -1;
    //总时长 ms
    long             m_Duration = 0;
    //数据流索引
    int              m_StreamIndex = -1;
    //锁和条件变量
    mutex               m_Mutex;
    condition_variable  m_Cond;
    thread             *m_Thread = nullptr;
    //seek position
    volatile float      m_SeekPosition = 0;
    volatile bool       m_SeekSuccess = false;
    //解码器状态
    volatile int  m_DecoderState = STATE_UNKNOWN;
    void* m_AVDecoderContext = nullptr;
    AVSyncCallback m_AVSyncCallback = nullptr;//用作音视频同步

public:

    //--------构造方法和析构方法-------------

    BaseDecoder()
    {};
    virtual ~BaseDecoder(){};

    //开始播放
    virtual void Start();
    //暂停播放
    virtual void Pause();
    //停止
    virtual void Stop();
    //获取时长
    virtual float GetDuration()
    {
        //ms to s
        return m_Duration * 1.0f / 1000;
    }
    //seek 到某个时间点播放
    virtual void SeekToPosition(float position);
    //当前播放的位置，用于更新进度条和音视频同步
    virtual float GetCurrentPosition();
    virtual void ClearCache()
    {};
    virtual void SetMessageCallback(void* context, MessageCallback callback)
    {
        m_MsgContext = context;
        m_MsgCallback = callback;
    }
    //设置音视频同步的回调
    virtual void SetAVSyncCallback(void* context, AVSyncCallback callback)
    {
        m_AVDecoderContext = context;
        m_AVSyncCallback = callback;
    }

protected:

    void * m_MsgContext = nullptr;
    MessageCallback m_MsgCallback = nullptr;
    virtual int Init(const char *url, AVMediaType mediaType);
    virtual void UnInit();
    virtual void OnDecoderReady() = 0;
    virtual void OnDecoderDone() = 0;
    //解码数据的回调
    virtual void OnFrameAvailable(AVFrame *frame) = 0;

    AVCodecContext *GetCodecContext() {
        return m_AVCodecContext;
    }
};

#endif //FFMPEGPLAYER_BASE_DECODER_H
