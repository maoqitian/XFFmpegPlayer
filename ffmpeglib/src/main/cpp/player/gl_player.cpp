//
// Created by maoqitian on 2020/12/18 0018.
// Description:
//

#include "gl_player.h"
#include "../opengl/drawer/proxy/def_drawer_proxy_impl.h"
#include "../render/audio/opensl_render.h"
#include "../utils/logger.h"

GLPlayer::GLPlayer(JNIEnv *jniEnv, jstring path) {

}

GLPlayer::~GLPlayer() {
    // 此处不需要 delete 成员指针
    // 在BaseDecoder 和 OpenGLRender 中的线程已经使用智能指针，会自动释放相关指针
}

void GLPlayer::SetSurface(jobject surface) {
    m_gl_render->SetSurface(surface);
}

void GLPlayer::PlayOrPause() {

}

void GLPlayer::Release() {
    m_gl_render->Stop();
    m_v_decoder->Stop();
    m_a_decoder->Stop();
}