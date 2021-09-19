//
// Created by lenovo on 2020/12/15 0015.
// 基础解码器

#include "base_decoder.h"
#include "utils/time.c"
#include "utils/logger.h"
//开始播放
void BaseDecoder::Start() {

    if(m_Thread == nullptr){
        //解码线程不存在创建
        StartDecodingThread();
    } else{
        //读写保护
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DecoderState = STATE_DECODING;
        m_Cond.notify_all();
    }

}

void BaseDecoder::Pause() {

    std::unique_lock<std::mutex> lock(m_Mutex);
    m_DecoderState = STATE_PAUSE;


}

void BaseDecoder::Stop() {
    LOGCATE("BaseDecoder::Stop");
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_DecoderState = STATE_STOP;
    m_Cond.notify_all();
}

void BaseDecoder::SeekToPosition(float position) {
    LOGCATE("BaseDecoder::SeekToPosition position=%f", position);
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_SeekPosition = position;
    m_DecoderState = STATE_DECODING;
    m_Cond.notify_all();
}

float BaseDecoder::GetCurrentPosition() {
    // ms
    return m_CurTimeStamp;
}

//初始化
int BaseDecoder::Init(const char *url, AVMediaType mediaType) {
    //输出当前初始化 url 和 类型
    LOGCATE("BaseDecoder::Init url=%s, mediaType=%d", url, mediaType);
    //地址转换
    strcpy(m_Url,url);
    m_MediaType = mediaType;
    return 0;
}

void BaseDecoder::UnInit() {
    LOGCATE("BaseDecoder::UnInit start m_MediaType=%d", m_MediaType);
    if (m_Thread){
        Stop();
        m_Thread ->join();
        delete m_Thread;
        m_Thread = nullptr;
    }

    LOGCATE("BaseDecoder::UnInit end m_MediaType=%d", m_MediaType);
}

//解码器初始化 使用 ffmpeg 提供方法
int BaseDecoder::InitFFDecoder() {
    return 0;
}

//释放解码器
void BaseDecoder::UnInitDecoder() {
    LOGCATE("BaseDecoder::UnInitDecoder");
    if(m_Frame != nullptr){
        av_frame_free(&m_Frame);
        m_Frame = nullptr;
    }

    if(m_Packet != nullptr) {
        av_packet_free(&m_Packet);
        m_Packet = nullptr;
    }

    if(m_AVCodecContext != nullptr) {
        avcodec_close(m_AVCodecContext);
        avcodec_free_context(&m_AVCodecContext);
        m_AVCodecContext = nullptr;
        m_AVCodec = nullptr;
    }

    if(m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }

}
//创建解码线程
void BaseDecoder::StartDecodingThread() {
    m_Thread = new thread(DoAVDecoding,this);
}

void BaseDecoder::DecodingLoop() {

}

//时间更新
void BaseDecoder::UpdateTimeStamp() {
    LOGCATE("BaseDecoder::UpdateTimeStamp");

    //读写保护
    std::unique_lock<std::mutex> lock(m_Mutex);

    if(m_Frame -> pkt_dts != AV_NOPTS_VALUE){
        m_CurTimeStamp = m_Frame -> pkt_dts;
    } else if(m_Frame -> pts != AV_NOPTS_VALUE){
        m_CurTimeStamp = m_Frame -> pts;
    } else{
        m_CurTimeStamp = 0;
    }

    m_CurTimeStamp = (int64_t)((m_CurTimeStamp * av_q2d(m_AVFormatContext -> streams[m_StreamIndex]->time_base))* 1000);

    if(m_SeekPosition > 0 && m_SeekSuccess){
        m_StartTimeStamp = GetSysCurrentTime() - m_CurTimeStamp;
        m_SeekPosition = 0;
        m_SeekSuccess = false;
    }
}

//音视频同步 系统时钟同步
long BaseDecoder::AVSync() {
    LOGCATE("BaseDecoder::AVSync");

    long currSysTime =  GetSysCurrentTime();
    //基于系统时钟计算从开始播放流逝的时间
    long elapsedTime = currSysTime - m_StartTimeStamp;

    if(m_MsgContext && m_MsgCallback && m_MediaType == AVMEDIA_TYPE_AUDIO){
        m_MsgCallback(m_MsgContext,MSG_DECODING_TIME,m_CurTimeStamp*1.0f/1000);
    }

    long  delay = 0;

    //和系统时钟同步
    if(m_CurTimeStamp > elapsedTime){
        //休眠时间 单位 ms
        auto sleepTime = static_cast<unsigned int >(m_CurTimeStamp - elapsedTime);

        //休眠时间不能过长，修正休眠时间
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;

        av_usleep(sleepTime * 1000);
    }

    delay = elapsedTime - m_CurTimeStamp;


    //    if(m_AVSyncCallback != nullptr && m_SeekPosition == 0) {
//        //视频向音频同步,或者音频向视频同步,视传进来的 m_AVSyncCallback 而定
//        long elapsedTime = m_AVSyncCallback(m_AVDecoderContext);
//        LOGCATE("DecoderBase::AVSync m_CurTimeStamp=%ld, elapsedTime=%ld", m_CurTimeStamp, elapsedTime);
//
//        if(m_CurTimeStamp > elapsedTime) {
//            //休眠时间
//            auto sleepTime = static_cast<unsigned int>(m_CurTimeStamp - elapsedTime);//ms
//            //限制休眠时间不能过长
//            sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD :  sleepTime;
//            av_usleep(sleepTime * 1000);
//        }
//        delay = elapsedTime - m_CurTimeStamp;
//    }

    return delay;
}

int BaseDecoder::DecodeOnePacket() {
    return 0;
}

void BaseDecoder::DoAVDecoding(BaseDecoder *decoder) {

    LOGCATE("BaseDecoder::DoAVDecoding");
    do {
        if(decoder -> InitFFDecoder() != 0){
            break;
        }
        decoder -> OnDecoderReady();
        decoder -> DecodingLoop();
    } while (false);

    decoder -> UnInitDecoder();
    decoder -> OnDecoderDone();

}
