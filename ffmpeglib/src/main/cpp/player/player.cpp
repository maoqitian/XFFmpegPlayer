//
// Created by maoqitian on 2020/12/16 0016.
// Description: 播放器
//

#include <render/audio/opensl_render.h>
#include "player.h"
#include "../render/video/native_render/native_render.h"

Player::Player(JNIEnv *jniEnv, jstring path, jobject surface) {

}

Player::~Player() {
    // 此处不需要 delete 成员指针
    // 在BaseDecoder中的线程已经使用智能指针，会自动释放
}

void Player::play() {

}

void Player::pause() {
    if (m_v_decoder != NULL) {
        m_v_decoder->Pause();
        m_a_decoder->Pause();
    }
}

