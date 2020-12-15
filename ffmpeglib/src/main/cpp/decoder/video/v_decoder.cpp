//
// Created by maoqitian on 2020/12/15 0015.
// 视频解码器

#include "v_decoder.h"


//构造函数参数直接传递给 BaseDecoder
VideoDecoder::VideoDecoder(JNIEnv *env, jstring path, bool for_synthesizer):BaseDecoder(env,path,for_synthesizer) {

}

//析构函数
VideoDecoder::~VideoDecoder(){
   delete m_video_render;
}

//准备解码环境
void VideoDecoder::Prepare(JNIEnv *env) {

    InitRender(env);
    InitBuffer();
    InitSws();
}

void VideoDecoder::InitRender(JNIEnv *env) {

}

//存放数据缓存初始化
void VideoDecoder::InitBuffer() {

    //初始化AVFrame
    m_rgb_frame = av_frame_alloc();
    //获取缓存大小
    int numBytes = av_image_get_buffer_size(DST_FORMAT,m_dst_w,m_dst_h,1);
    //分配内存
    m_buf_for_rgb_frame = (uint8_t*) av_malloc(numBytes* sizeof(uint8_t));
    //将内存分配给RgbFrame，并将内存格式化为三个通道后，分别保存其地址
    av_image_fill_arrays(m_rgb_frame ->data,m_rgb_frame->linesize,
            m_buf_for_rgb_frame,DST_FORMAT,m_dst_w,m_dst_h,1);
}

//数据转换工具初始化
//视频解码出来以后，数据格式是 YUV ，而屏幕显示的时候需要 RGBA，因此视频解码器中，需要对数据做一层转换
//swresampel 工具包 sws_scale 既可以实现数据格式的转化，同时可以对画面宽高进行缩放
void VideoDecoder::InitSws() {
    //初始化格式转换工具
    m_sws_ctx = sws_getContext(width(),height(),video_pixel_format(),
            m_dst_w,m_dst_h,DST_FORMAT,SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
}


//释放相关资源
void VideoDecoder::Release() {

    LOGE(TAG,"VIDEO release")

    if(m_rgb_frame != nullptr){
        av_frame_free(&m_rgb_frame);
        m_rgb_frame = nullptr;
    }
    if(m_buf_for_rgb_frame != nullptr){
        free(m_buf_for_rgb_frame);
        m_buf_for_rgb_frame = nullptr
    }
    if(m_sws_ctx != nullptr){
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = nullptr;
    }
    if(m_video_render != nullptr){
        m_video_render->ReleaseRender();
        m_video_render = nullptr;
    }
}

bool VideoDecoder::NeedLoopDecode() {
    return true;
}




