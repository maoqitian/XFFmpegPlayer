//
// Created by maoqitian on 2020/12/16 0016.
// Description: 播放器
//

#ifndef FFMPEGPLAYER_PLAYER_H
#define FFMPEGPLAYER_PLAYER_H

#include <decoder/audio/a_decoder.h>
#include "../decoder/video/v_decoder.h"
#include "../render/video/video_render.h"

class Player {
private:
    //视频解码器 视频渲染器
    VideoDecoder *m_v_decoder;
    VideoRender *m_v_render;

    AudioDecoder *m_a_decoder;
    AudioRender *m_a_render;

public:
    //构造函数
    Player(JNIEnv *jniEnv, jstring path, jobject surface);
    //析构函数
    ~Player();


    void play();
    void pause();
};


#endif //FFMPEGPLAYER_PLAYER_H
