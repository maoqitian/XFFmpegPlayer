//
// Created by maoqitian on 2020/12/15 0015.
// 视频解码器

#ifndef FFMPEGPLAYER_V_DECODER_H
#define FFMPEGPLAYER_V_DECODER_H
#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <base_decoder.h>
#include <render/video/video_render.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/jni.h>
}

class VideoDecoder  : public BaseDecoder{

public:
    //构造函数
    VideoDecoder(char *url){
        Init(url,AVMEDIA_TYPE_VIDEO);
    }
    //析构函数
    ~VideoDecoder(){
      UnInit();
    }

    int GetVideoWidth()
    {
        return m_VideoWidth;
    }
    int GetVideoHeight()
    {
        return m_VideoHeight;
    }

    void SetVideoRender(VideoRender *videoRender)
    {
        m_VideoRender = videoRender;
    }

private:
    virtual void OnDecoderReady();
    virtual void OnDecoderDone();
    virtual void OnFrameAvailable(AVFrame *frame);

    const AVPixelFormat DST_PIXEL_FORMAT = AV_PIX_FMT_RGBA;

    int m_VideoWidth = 0;
    int m_VideoHeight = 0;

    int m_RenderWidth = 0;
    int m_RenderHeight = 0;

    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;

    VideoRender *m_VideoRender = nullptr;
    SwsContext *m_SwsContext = nullptr;
    //SingleVideoRecorder *m_pVideoRecorder = nullptr;
};


#endif //FFMPEGPLAYER_V_DECODER_H
