//
// Created by lenovo on 2020/12/15 0015.
// 基础解码器

#include "base_decoder.h"
#include "utils/time.c"
//构造函数
BaseDecoder::BaseDecoder(JNIEnv *env, jstring path,bool for_synthesizer) :m_for_synthesizer(for_synthesizer){
    Init(env,path);
    CreateDecodeThread();
}

//析构函数 资源释放
BaseDecoder::~BaseDecoder() {
    if (m_format_ctx != nullptr) delete m_format_ctx;
    if (m_codec_ctx != nullptr) delete m_codec_ctx;
    if (m_frame != nullptr) delete m_frame;
    if (m_packet != nullptr) delete m_packet;
}

//初始化
void BaseDecoder::Init(JNIEnv *env, jstring path){
     m_path_ref = env->NewGlobalRef(path);
     m_path = env->GetStringUTFChars(path,nullptr);
     //获取 JVM 创建线程做准备
     env->GetJavaVM(&m_jvm_for_thread);
}

//解码线程创建
void BaseDecoder::CreateDecodeThread() {
     //智能指针 线程结束自动删除 防止内存泄漏
     std::shared_ptr<BaseDecoder>that(this);
     //创建线程
     //std::thread t(静态方法, 静态方法参数);
     std::thread t(Decode,that);
     t.detach();
}

//封装解码流程
void BaseDecoder::Decode(std::shared_ptr<BaseDecoder> that) {

    JNIEnv * env;

    //将线程加入到虚拟机
    if(that->m_jvm_for_thread->AttachCurrentThread(&env, nullptr)!=JNI_OK){
        ALOGE( "Fail to Init decode thread");
        return;
    }

    //初始化解码器
    that->InitFFmpegDecoder(env);
    //分配解码帧数据内存
    that->AllocFrameBuffer();
    //回调，通知解码器初始化完毕
    that->Prepare(env);
    //进入解码循环
    that->LoopDecode();
    //退出解码
    that->DoneDecode(env);
    //解除线程和JVM关联
    that->m_jvm_for_thread->DetachCurrentThread();
}

//编码器初始化
void BaseDecoder::InitFFmpegDecoder(JNIEnv *env) {

    //1.初始化上下文
    m_format_ctx = avformat_alloc_context();
    //2.打开文件
    if(avformat_open_input(&m_format_ctx,m_path, nullptr, nullptr)!=0){
       ALOGE("Fail to open file :",m_path);
       DoneDecode(env);
       return;
    }
    //3.获取音视频流信息
    if(avformat_find_stream_info(m_format_ctx, nullptr) <0){
        ALOGE("Fail to find stream info");
        DoneDecode(env);
        return;
    }
    //4.查找编码器
    //4.1 获取视频流的索引
    int vIdx = -1;
    for(int i = 0; i < m_format_ctx->nb_streams;++i){
        if(m_format_ctx->streams[i]->codecpar->codec_type == GetMediaType()){
            vIdx = i;
            break;
        }
    }
    if(vIdx == -1){
        ALOGE("Fail to find stream index");
        DoneDecode(env);
        return;
    }
    m_stream_index = vIdx;
    //4.2 获取编码器参数
    AVCodecParameters *codecPar = m_format_ctx->streams[vIdx]->codecpar;
    //4.3 获取解码器
    m_codec = avcodec_find_decoder(codecPar->codec_id);
    //4.4 获取解码器上下文
    m_codec_ctx = avcodec_alloc_context3(m_codec);
    if(avcodec_parameters_to_context(m_codec_ctx,codecPar) != 0){
        ALOGE("Fail to obtain av codec context");
        DoneDecode(env);
        return;
    }
    //5 打开解码器
    if(avcodec_open2(m_codec_ctx, m_codec, nullptr)<0){
        ALOGE("Fail to open av codec");
        DoneDecode(env);
        return;
    }

    m_duration = (long)((float) m_format_ctx -> duration/AV_TIME_BASE*1000);

    ALOGI("Decoder init success");
}

//待解码和解码数据结构
void BaseDecoder::AllocFrameBuffer() {

    //1.初始化 AVPacket,存放解码前的数据
    m_packet = av_packet_alloc();
    //2.初始化AVFrame,存放解码后的数据
    m_frame = av_frame_alloc();
}
//解码循环 音画同步
void BaseDecoder::LoopDecode() {

    if(STOP == m_state) m_state = START;

    LOG_INFO(TAG, LogSpec(), "Start loop decode")

    while (1){
        if(m_state!=DECODING && m_state!=START && m_state!=STOP){
            Wait();
            //恢复同步起始时间，去除等待流失时间
            m_started_t = GetCurMsTime() -m_cur_t_s;
        }
        if(m_state == STOP) break;

        if(-1 == m_started_t){
            m_started_t = GetCurMsTime();
        }
        //解码一帧数据
        if(DecodeOneFrame() != nullptr){
            //同步时间
            SyncRender();
            Render(m_frame);
            if(m_state == START){
                m_state = PAUSE;
            }
        } else{
            LOG_INFO(TAG, LogSpec(), "m_state = %d",m_state)
            if(ForSynthesizer()){
               m_state = STOP;
            } else{
               m_state = FINISH;
            }
        }
    }

}
//解码一帧数据
AVFrame *BaseDecoder::DecodeOneFrame() {
    int ret = av_read_frame(m_format_ctx,m_packet);
    while (ret == 0){
        if(m_packet->stream_index == m_stream_index){
            switch (avcodec_send_packet(m_codec_ctx,m_packet)){
                case AVERROR_EOF:{
                    av_packet_unref(m_packet);
                    LOG_ERROR(TAG, LogSpec(), "Decode error: %s", av_err2str(AVERROR_EOF))
                    return nullptr;//解码结束
                }
                case AVERROR(EAGAIN):{
                    LOG_ERROR(TAG, LogSpec(), "Decode error: %s", av_err2str(AVERROR(EAGAIN)))
                    break;
                }
                case AVERROR(EINVAL):{
                    LOG_ERROR(TAG, LogSpec(), "Decode error: %s", av_err2str(AVERROR(EINVAL)))
                    break;
                }
                case AVERROR(ENOMEM):{
                    LOG_ERROR(TAG, LogSpec(), "Decode error: %s", av_err2str(AVERROR(ENOMEM)))
                    break;
                }
                default:
                    break;
            }
            int result = avcodec_receive_frame(m_codec_ctx,m_frame);
            if(result == 0){
                ObtainTimeStamp();
                av_packet_unref(m_packet);
                //返回最终解码好的数据
                return m_frame;
            } else{
                LOG_INFO(TAG, LogSpec(), "Receive frame error result: %d", av_err2str(AVERROR(result)))
            }
        }
        //释放资源 packet
        av_packet_unref(m_packet);
        ret = av_read_frame(m_format_ctx,m_packet);
    }
    //释放资源 防止内存泄漏
    av_packet_unref(m_packet);
    LOGI(TAG, "ret = %d", ret)
    return nullptr;
}

//等待
void BaseDecoder::Wait(long second, long ms) {
     pthread_mutex_lock(&m_mutex);
     if(second > 0 || ms > 0){
         timeval now;
         timespec outtime;
         gettimeofday(&now, nullptr);
         int64_t destNSec = now.tv_usec * 1000 + ms * 1000000;
         outtime.tv_sec = static_cast<__kernel_time_t>(now.tv_sec + second + destNSec / 1000000000);
         outtime.tv_nsec = static_cast<long>(destNSec % 1000000000);
         pthread_cond_timedwait(&m_cond, &m_mutex, &outtime);
     } else{
         pthread_cond_wait(&m_cond,&m_mutex);
     }
     pthread_mutex_lock(&m_mutex);
}

//解码完毕 释放资源
void BaseDecoder::DoneDecode(JNIEnv *env) {
    LOG_INFO(TAG, LogSpec(), "Decode done and decoder release")
    //缓存释放
    if(m_packet != nullptr){
        av_packet_free(&m_packet);
    }
    //关闭解码器
    if(m_frame!= nullptr){
        av_frame_free(&m_frame);
    }
    //关闭流输入
    if(m_format_ctx != nullptr){
        avformat_close_input(&m_format_ctx);
        avformat_free_context(m_format_ctx);
    }
    //释放转换参数
    if(m_path_ref != nullptr && m_path != nullptr){
        env -> ReleaseStringUTFChars((jstring) m_path_ref,m_path);
        env -> DeleteGlobalRef(m_path_ref);
    }
    //通知子类释放资源
    Release();
}

//当前帧时间戳
void BaseDecoder::ObtainTimeStamp() {
   if(m_frame->pkt_dts != AV_NOPTS_VALUE){
       m_cur_t_s = m_packet->dts;
   } else if(m_frame -> pts != AV_NOPTS_VALUE){
       m_cur_t_s = m_frame->pts;
   } else{
       m_cur_t_s = 0;
   }
   m_cur_t_s = (ino64_t)((m_cur_t_s*av_q2d(m_format_ctx->streams[m_stream_index]->time_base))*1000);
}

//时间同步
void BaseDecoder::SyncRender() {
      //只有解码播放才同步
      if(ForSynthesizer())return;

      int64_t ct = GetCurMsTime();
      int64_t passTime = ct - m_started_t;
      if(m_cur_t_s > passTime){
          av_usleep((unsigned int)((m_cur_t_s - passTime)*1000));
      }
}
//解码恢复
void BaseDecoder::SendSignal() {
    pthread_mutex_lock(&m_mutex);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}

void BaseDecoder::GoOn() {
      m_state = DECODING;
      SendSignal();
}

void BaseDecoder::Pause() {
      m_state = PAUSE;
}

void BaseDecoder::Stop() {
      m_state = STOP;
      SendSignal();
}

bool BaseDecoder::IsRunning() {
    return DECODING == m_state;
}

long BaseDecoder::GetDuration() {
    return m_duration;
}

long BaseDecoder::GetCurPos() {
    return (long) m_cur_t_s;
}




