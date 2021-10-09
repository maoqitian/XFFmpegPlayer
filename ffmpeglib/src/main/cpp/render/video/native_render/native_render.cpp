//
// Created by maoqitian on 2020/12/16 0016.
// Description: 本地窗口 ANativeWindow 渲染器 surface
// 分为四个步骤

#include <utils/logger.h>
#include "native_render.h"

NativeRender::NativeRender(JNIEnv *env, jobject surface) : VideoRender(VIDEO_RENDER_ANWINDOW) {
    //1. 利用 Java 层 SurfaceView 传下来的 Surface 对象，获取 ANativeWindow
    m_NativeWindow = ANativeWindow_fromSurface(env,surface);
}

//析构函数中释放资源
NativeRender::~NativeRender() {
    //4. 释放 ANativeWindow
    if (m_NativeWindow)
        ANativeWindow_release(m_NativeWindow);

}

//初始化窗口
void NativeRender::Init(int videoWidth, int videoHeight, int *dstSize) {

    LOGCATE("NativeRender::Init m_NativeWindow=%p, video[w,h]=[%d, %d]", m_NativeWindow, videoWidth, videoHeight);

    if (m_NativeWindow == nullptr) return;

    int windowWidth = ANativeWindow_getWidth(m_NativeWindow);
    int windowHeight = ANativeWindow_getHeight(m_NativeWindow);

    //修正宽高
    if (windowWidth < windowHeight * videoWidth / videoHeight) {
        m_DstWidth = windowWidth;
        m_DstHeight = windowWidth * videoHeight / videoWidth;
    } else {
        m_DstWidth = windowHeight * videoWidth / videoHeight;
        m_DstHeight = windowHeight;
    }
    LOGCATE("NativeRender::Init window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]", windowWidth, windowHeight, m_DstWidth, m_DstHeight);

    //2. 设置渲染区域和输入格式
    ANativeWindow_setBuffersGeometry(m_NativeWindow,m_DstWidth,
                                     m_DstHeight,WINDOW_FORMAT_RGBA_8888);

    dstSize[0] = m_DstWidth;
    dstSize[1] = m_DstHeight;
}

//开始渲染
void NativeRender::RenderVideoFrame(NativeImage *pImage) {

    //3. 渲染
    if (m_NativeWindow == nullptr || pImage == nullptr) return;
    //锁定当前 Window ，获取屏幕缓冲区 Buffer 的指针
    ANativeWindow_lock(m_NativeWindow,&m_NativeWindowBuffer, nullptr);

    uint8_t *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = pImage->width * 4;//RGBA
    int dstLineSize = m_NativeWindowBuffer.stride * 4;

    for (int i = 0; i < m_DstHeight; ++i) {
        //一行一行地拷贝图像数据
        memcpy(dstBuffer + i * dstLineSize, pImage->ppPlane[0] + i * srcLineSize, srcLineSize);
    }

    //解锁当前 Window ，渲染缓冲区数据
    ANativeWindow_unlockAndPost(m_NativeWindow);


}

void NativeRender::UnInit() {

}
