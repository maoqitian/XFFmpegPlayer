//
// Created by maoqitian on 2020/12/16 0016.
// Description: 播放器
//

#include <render/audio/opensl_render.h>
#include "player.h"
#include "../render/video/native_render/native_render.h"

Player::Player(JNIEnv *jniEnv, jstring path, jobject surface) {

    m_v_decoder = new VideoDecoder(jniEnv, path);
    m_v_render = new NativeRender(jniEnv, surface);
    m_v_decoder->SetRender(m_v_render);

    // 音频解码
    m_a_decoder = new AudioDecoder(jniEnv, path, false);
    m_a_render = new OpenSLRender();
    m_a_decoder->SetRender(m_a_render);

}

Player::~Player() {
    // 此处不需要 delete 成员指针
    // 在BaseDecoder中的线程已经使用智能指针，会自动释放
}

void Player::play() {
    if (m_v_decoder != NULL) {
        m_v_decoder->GoOn();
        m_a_decoder->GoOn();
    }
}

void Player::pause() {
    if (m_v_decoder != NULL) {
        m_v_decoder->Pause();
        m_a_decoder->Pause();
    }
}

