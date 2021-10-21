//
// Created by maoqitian on 2020/12/15 0015.
// 视频解码器
// ANativeWindow 仅支持 RGB 类型的图像数据，所以还需要利用 libswscale 库将解码后的 YUV 数据转成 RGB
// 分为四个步骤
#include "v_decoder.h"

//准备解码
void VideoDecoder::OnDecoderReady() {
    LOGCATE("VideoDecoder::OnDecoderReady");

    //利用 libswscale 库将对图像进行格式转换

    //1. 分配存储 RGB 图像的 buffer
    m_VideoWidth = GetCodecContext() -> width;
    m_VideoHeight = GetCodecContext()-> height;

    if (m_MsgContext && m_MsgCallback){
        m_MsgCallback(m_MsgContext,MSG_DECODER_READY,0);
    }

    if (m_VideoRender != nullptr){
        int dstSize[2] = {0};
        //视频渲染初始化
        m_VideoRender -> Init(m_VideoWidth, m_VideoHeight, dstSize);

        m_RenderWidth = dstSize[0];
        m_RenderHeight = dstSize[1];

        if(m_VideoRender-> GetRenderType() == VIDEO_RENDER_ANWINDOW) {
            int fps = 25;
            long videoBitRate = m_RenderWidth * m_RenderHeight * fps * 0.2;
            //录屏
            //m_pVideoRecorder = new SingleVideoRecorder("/sdcard/learnffmpeg_output.mp4", m_RenderWidth, m_RenderHeight, videoBitRate, fps);
            //m_pVideoRecorder->StartRecord();
        }

        m_RGBAFrame = av_frame_alloc();
        //计算 Buffer 的大小
        int bufferSize = av_image_get_buffer_size(DST_PIXEL_FORMAT, m_RenderWidth, m_RenderHeight, 1);
        //为 m_RGBAFrame 分配空间
        m_FrameBuffer = (uint8_t *) av_malloc(bufferSize * sizeof(uint8_t));
        av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                             m_FrameBuffer, DST_PIXEL_FORMAT, m_RenderWidth, m_RenderHeight, 1);
        //2. 获取转换的上下文
        m_SwsContext = sws_getContext(m_VideoWidth, m_VideoHeight, GetCodecContext()->pix_fmt,
                                      m_RenderWidth, m_RenderHeight, DST_PIXEL_FORMAT,
                                      SWS_FAST_BILINEAR, NULL, NULL, NULL);
    } else{
        LOGCATE("VideoDecoder::OnDecoderReady m_VideoRender == null");
    }
}

//4 解码结束 释放资源
void VideoDecoder::OnDecoderDone() {
    LOGCATE("VideoDecoder::OnDecoderDone");

    if(m_MsgContext && m_MsgCallback){
        m_MsgCallback(m_MsgContext,MSG_DECODER_DONE,0);
    }

    if (m_VideoRender){
        m_VideoRender->UnInit();
    }

    if(m_RGBAFrame != nullptr) {
        av_frame_free(&m_RGBAFrame);
        m_RGBAFrame = nullptr;
    }

    if(m_FrameBuffer != nullptr) {
        free(m_FrameBuffer);
        m_FrameBuffer = nullptr;
    }

    if(m_SwsContext != nullptr) {
        sws_freeContext(m_SwsContext);
        m_SwsContext = nullptr;
    }
//录屏
//    if(m_pVideoRecorder != nullptr) {
//        m_pVideoRecorder->StopRecord();
//        delete m_pVideoRecorder;
//        m_pVideoRecorder = nullptr;
//    }

}

//一帧画面设置
void VideoDecoder::OnFrameAvailable(AVFrame *frame) {

    LOGCATE("VideoDecoder::OnFrameAvailable frame=%p", frame);

    if (m_VideoRender != nullptr && frame != nullptr){
        NativeImage image;
        LOGCATE("VideoDecoder::OnFrameAvailable frame[w,h]=[%d, %d],format=%d,[line0,line1,line2]=[%d, %d, %d]", frame->width, frame->height, GetCodecContext()->pix_fmt, frame->linesize[0], frame->linesize[1],frame->linesize[2]);

        if(m_VideoRender->GetRenderType() == VIDEO_RENDER_ANWINDOW){
            //如果是 ANativeWindow 渲染 YUV 转 RGB
            // 3. 格式转换
            sws_scale(m_SwsContext, frame->data, frame->linesize,0,
                      m_VideoHeight,m_RGBAFrame->data,m_RGBAFrame->linesize);

            image.format = IMAGE_FORMAT_RGBA;
            image.width = m_RenderWidth;
            image.height = m_VideoHeight;
            image.ppPlane[0] = m_RGBAFrame->data[0];
            image.pLineSize[0] = image.width * 4;
        } else if(GetCodecContext()->pix_fmt == AV_PIX_FMT_YUV420P || GetCodecContext()->pix_fmt == AV_PIX_FMT_YUVJ420P){
            image.format = IMAGE_FORMAT_I420;
            image.width = frame->width;
            image.height = frame->height;
            image.pLineSize[0] = frame->linesize[0];
            image.pLineSize[1] = frame->linesize[1];
            image.pLineSize[2] = frame->linesize[2];
            image.ppPlane[0] = frame->data[0];
            image.ppPlane[1] = frame->data[1];
            image.ppPlane[2] = frame->data[2];
            if(frame->data[0] && frame->data[1] && !frame->data[2] && frame->linesize[0] == frame->linesize[1] && frame->linesize[2] == 0) {
                // on some android device, output of h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
                image.format = IMAGE_FORMAT_NV12;
            }
        } else if(GetCodecContext()->pix_fmt == AV_PIX_FMT_NV12){
            image.format = IMAGE_FORMAT_NV12;
            image.width = frame->width;
            image.height = frame->height;
            image.pLineSize[0] = frame->linesize[0];
            image.pLineSize[1] = frame->linesize[1];
            image.ppPlane[0] = frame->data[0];
            image.ppPlane[1] = frame->data[1];
        } else if(GetCodecContext()->pix_fmt == AV_PIX_FMT_NV21){
            image.format = IMAGE_FORMAT_NV21;
            image.width = frame->width;
            image.height = frame->height;
            image.pLineSize[0] = frame->linesize[0];
            image.pLineSize[1] = frame->linesize[1];
            image.ppPlane[0] = frame->data[0];
            image.ppPlane[1] = frame->data[1];
        } else if(GetCodecContext()->pix_fmt == AV_PIX_FMT_RGBA){
            image.format = IMAGE_FORMAT_RGBA;
            image.width = frame->width;
            image.height = frame->height;
            image.pLineSize[0] = frame->linesize[0];
            image.ppPlane[0] = frame->data[0];
        } else{
            sws_scale(m_SwsContext, frame->data, frame->linesize, 0,
                      m_VideoHeight, m_RGBAFrame->data, m_RGBAFrame->linesize);
            image.format = IMAGE_FORMAT_RGBA;
            image.width = m_RenderWidth;
            image.height = m_RenderHeight;
            image.ppPlane[0] = m_RGBAFrame->data[0];
            image.pLineSize[0] = image.width * 4;
        }

        m_VideoRender -> RenderVideoFrame(&image);

        //屏幕录制
        /*if(m_pVideoRecorder != nullptr) {
            m_pVideoRecorder->OnFrame2Encode(&image);
        }*/
    }

    //回调
    if(m_MsgContext && m_MsgCallback)
        m_MsgCallback(m_MsgContext, MSG_REQUEST_RENDER, 0);

}
