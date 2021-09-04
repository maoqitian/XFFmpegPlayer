//
// Created by maoqitian on 2020/12/16 0016.
// 视频渲染器 相当于抽象接口

#ifndef FFMPEGPLAYER_VIDEO_RENDER_H
#define FFMPEGPLAYER_VIDEO_RENDER_H

#include "stdint.h"
#include "../one_frame.h"
#include <jni.h>

class VideoRender  {
public:
    virtual void InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) = 0;
    virtual void Render(OneFrame *one_frame) = 0;
    virtual void ReleaseRender() = 0;
};


#endif //FFMPEGPLAYER_VIDEO_RENDER_H
