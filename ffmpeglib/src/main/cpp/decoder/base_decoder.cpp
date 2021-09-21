//
// Created by maoqitian on 2020/12/15 0015.
// 基础解码器 提供解码基础方法

#include "base_decoder.h"
#include "utils/time.c"
#include "utils/logger.h"
//开始播放
void BaseDecoder::Start() {

    if(m_Thread == nullptr){
        //解码线程不存在创建
        StartDecodingThread();
    } else{
        //获取锁 读写保护
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
    int result = -1;
    do {
       //1 创建封装格式上下文
       m_AVFormatContext = avformat_alloc_context();

       //2 打开文件
       if(avformat_open_input(&m_AVFormatContext,m_Url,NULL,NULL)!=0){
           LOGCATE("BaseDecoder::InitFFDecoder avformat_open_input fail.");
           break;
       }

       //3 获取音视频流信息
       if(avformat_find_stream_info(m_AVFormatContext,NULL) < 0){
           LOGCATE("BaseDecoder::InitFFDecoder avformat_find_stream_info fail.");
           break;
       }

       //4获取音视频流索引
        for (int i = 0; i < m_AVFormatContext-> nb_streams; i++) {
            if(m_AVFormatContext -> streams[i]->codecpar->codec_type == m_MediaType){
                m_StreamIndex = i;
                break;
            }
        }
        //没找到音视频流索引 退出
        if (m_StreamIndex == -1){
            LOGCATE("BaseDecoder::InitFFDecoder Fail to find stream index.");
            break;
        }

        //5. 获取解码参数
        AVCodecParameters *codecParameters = m_AVFormatContext-> streams[m_StreamIndex]->codecpar;

        //6. 获取解码器
        m_AVCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (m_AVCodec == nullptr){
            LOGCATE("BaseDecoder::InitFFDecoder avcodec_find_decoder fail.");
            break;
        }

        //7. 创建解码器上下文
        m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
        if (avcodec_parameters_to_context(m_AVCodecContext,codecParameters)!=0){
            LOGCATE("BaseDecoder::InitFFDecoder avcodec_parameters_to_context fail.");
            break;
        }

        //设置解码参数 每次读取buff大小 超时时间 等
        AVDictionary *pAVDictionary = nullptr;
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

        //8. 打开解码器
        result = avcodec_open2(m_AVCodecContext,m_AVCodec,&pAVDictionary);
        if(result < 0){
            LOGCATE("BaseDecoder::InitFFDecoder avcodec_open2 fail.result=%d", result);
            break;
        }
        //解码成功
        result = 0;

        //总时间长度 ms
        m_Duration = m_AVFormatContext->duration/AV_TIME_BASE * 1000;

        // AVPacket 存放编码数据
        m_Packet = av_packet_alloc();
        //解码后的数据 帧
        m_Frame = av_frame_alloc();
    } while (false);

    //解码失败
    if(result != 0 && m_MsgContext && m_MsgCallback){
        m_MsgCallback(m_MsgContext,MSG_DECODER_INIT_ERROR,0);
    }

    return result;
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
    LOGCATE("BaseDecoder::DecodingLoop start, m_MediaType=%d", m_MediaType);

    {
        //获取锁 读写保护
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DecoderState = STATE_DECODING;
        lock.unlock();
    }

    for(;;){
        while (m_DecoderState == STATE_PAUSE){
            //暂停
            std::unique_lock<std::mutex> lock(m_Mutex);
            LOGCATE("BaseDecoder::DecodingLoop waiting, m_MediaType=%d", m_MediaType);
            m_Cond.wait_for(lock,std::chrono::milliseconds(10));
            m_StartTimeStamp = GetSysCurrentTime() - m_CurTimeStamp;
        }

        if(m_DecoderState == STATE_STOP){
            //停止状态 直接退出
            break;
        }

        if(m_StartTimeStamp == -1){
            m_StartTimeStamp = GetSysCurrentTime();
        }

        //解码结束 暂停解码
        if(DecodeOnePacket() != 0){
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_DecoderState = STATE_PAUSE;
        }
    }

    LOGCATE("BaseDecoder::DecodingLoop end.");
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

//解码一个packet编码数据
int BaseDecoder::DecodeOnePacket() {
    LOGCATE("BaseDecoder::DecodeOnePacket  m_MediaType=%d", m_MediaType);
    if(m_SeekPosition > 0){
        //seek to frame 快进处理
        int64_t seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);//微秒
        int64_t seek_min = INT64_MIN;
        int64_t seek_max = INT64_MAX;
        int seek_ret = avformat_seek_file(m_AVFormatContext,-1,seek_min,seek_target,seek_max,0);
        if (seek_ret < 0){
            m_SeekSuccess = false;
            LOGCATE("BaseDecoder::DecodeOneFrame error while seeking m_MediaType=%d", m_MediaType);
        } else{
            if(-1 != m_StreamIndex){
                avcodec_flush_buffers(m_AVCodecContext);
            }
            ClearCache();
            m_SeekSuccess = true;
            LOGCATE("BaseDecoder::DecodeOneFrame seekFrame pos=%f, m_MediaType=%d", m_SeekPosition, m_MediaType);
        }
    }

    int result = av_read_frame(m_AVFormatContext,m_Packet);
    while (result == 0){
        //读取每一帧成功
        if(m_Packet -> stream_index == m_StreamIndex){
            //            UpdateTimeStamp(m_Packet);
//            if(AVSync() > DELAY_THRESHOLD && m_CurTimeStamp > DELAY_THRESHOLD)
//            {
//                result = 0;
//                goto __EXIT;
//            }

            if(avcodec_send_packet(m_AVCodecContext, m_Packet) == AVERROR_EOF) {
                //解码结束
                result = -1;
                goto __EXIT;
            }
            //一个 packet 包含多少 frame
            int frameCount = 0;
            while (avcodec_receive_frame(m_AVCodecContext,m_Frame) == 0){
                //时间戳更新
                UpdateTimeStamp();
                //音视频时间同步
                AVSync();
                //画面渲染
                LOGCATE("BaseDecoder::DecodeOnePacket 000 m_MediaType=%d", m_MediaType);
                OnFrameAvailable(m_Frame);
                LOGCATE("BaseDecoder::DecodeOnePacket 001 m_MediaType=%d", m_MediaType);
                frameCount++;
            }
            LOGCATE("BaseDecoder::DecodeOneFrame frameCount=%d", frameCount);
            //判断一个 packet 是否解码完成
            if(frameCount > 0) {
                result = 0;
                goto __EXIT;
            }
        }

        av_packet_unref(m_Packet);

        result = av_read_frame(m_AVFormatContext,m_Packet);

    }
    __EXIT:
    av_packet_unref(m_Packet);
    return result;
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
