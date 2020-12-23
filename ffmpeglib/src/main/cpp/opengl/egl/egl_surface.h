//
// Created by maoqitian on 2020/12/18 0018.
// Description: OpenGL EGL窗口
//

#ifndef FFMPEGPLAYER_EGL_SURFACE_H
#define FFMPEGPLAYER_EGL_SURFACE_H

#include <android/native_window.h>
#include <EGL/egl.h>
#include "elg_core.h"


class EglSurface {
private:

    const char *TAG = "EglSurface";

    ANativeWindow *m_native_window = NULL;

    EglCore *m_core;

    EGLSurface m_surface;


public:
    EglSurface();
    ~EglSurface();

    bool Init();
    void CreateEglSurface(ANativeWindow *native_window, int width, int height);
    void MakeCurrent();
    void SwapBuffers();
    void DestroyEglSurface();
    void Release();
};


#endif //FFMPEGPLAYER_EGL_SURFACE_H
