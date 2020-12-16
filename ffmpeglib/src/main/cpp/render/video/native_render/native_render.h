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
    const char *TAG = "NativeRender";

    // Surface引用，必须使用引用，否则无法在线程中操作
    jobject m_surface_ref = NULL;

    // 存放输出到屏幕的缓存数据
    ANativeWindow_Buffer m_out_buffer;

    // 本地窗口
    ANativeWindow *m_native_window = NULL;

    //显示的目标宽
    int m_dst_w;

    //显示的目标高
    int m_dst_h;
public:
    //构造函数
    NativeRender(JNIEnv *env, jobject surface);
    //析构函数
    ~NativeRender();
    //VideoRender 接口方法
    void InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) override ;
    void Render(OneFrame *one_frame) override ;
    void ReleaseRender() override ;
};

#endif //FFMPEGPLAYER_NATIVE_RENDER_H
