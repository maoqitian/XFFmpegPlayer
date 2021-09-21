//
// Created by maoqitian on 2020/12/16 0016.
// 视频渲染器 相当于抽象接口

#ifndef FFMPEGPLAYER_VIDEO_RENDER_H
#define FFMPEGPLAYER_VIDEO_RENDER_H

#include "stdint.h"
#include "../one_frame.h"
#include <jni.h>

//使用渲染模式
#define VIDEO_RENDER_OPENGL             0
#define VIDEO_RENDER_ANWINDOW           1
#define VIDEO_RENDER_3D_VR              2

#include "ImageDef.h"

class VideoRender {
public:
    VideoRender(int type){
        m_RenderType = type;
    }
    virtual ~VideoRender(){}
    virtual void Init(int videoWidth, int videoHeight, int *dstSize) = 0;
    virtual void RenderVideoFrame(NativeImage *pImage) = 0;
    virtual void UnInit() = 0;

    int GetRenderType() {
        return m_RenderType;
    }
private:
    int m_RenderType = VIDEO_RENDER_ANWINDOW;
};



#endif //FFMPEGPLAYER_VIDEO_RENDER_H
