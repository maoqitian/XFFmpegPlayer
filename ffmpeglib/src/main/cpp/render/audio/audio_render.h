//
// Created by maoqitian on 2020/12/17 0017.
// Description: 音频渲染接口 纯虚函数
//

#ifndef FFMPEGPLAYER_AUDIO_RENDER_H
#define FFMPEGPLAYER_AUDIO_RENDER_H

#include <cstdint>

class AudioRender {
public:
    virtual void InitRender() = 0;
    virtual void Render(uint8_t *pcm, int size) = 0;
    virtual void ReleaseRender() = 0;
    virtual ~AudioRender() {}
};
#endif //FFMPEGPLAYER_AUDIO_RENDER_H
