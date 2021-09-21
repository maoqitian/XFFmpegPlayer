//
// Created by maoqitian on 2020/12/16 0016.
// Description: 本地窗口渲染器 surface
//

#ifndef FFMPEGPLAYER_NATIVE_RENDER_H
#define FFMPEGPLAYER_NATIVE_RENDER_H


#include <render/video/video_render.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

extern "C"{
#include <libavutil/mem.h>
};

class NativeRender : public VideoRender{
public:
    NativeRender(JNIEnv *env, jobject surface);
    virtual ~NativeRender();
    virtual void Init(int videoWidth, int videoHeight, int *dstSize);
    virtual void RenderVideoFrame(NativeImage *pImage);
    virtual void UnInit();

private:
    ANativeWindow_Buffer m_NativeWindowBuffer;
    ANativeWindow *m_NativeWindow = nullptr;
    int m_DstWidth;
    int m_DstHeight;
};

#endif //FFMPEGPLAYER_NATIVE_RENDER_H
