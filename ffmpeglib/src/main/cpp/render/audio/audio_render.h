//
// Created by maoqitian on 2020/12/17 0017.
// Description: 音频渲染接口 纯虚函数
//

#ifndef FFMPEGPLAYER_AUDIO_RENDER_H
#define FFMPEGPLAYER_AUDIO_RENDER_H

#include <cstdint>
#include <cstring>
#include <cstdlib>

class AudioFrame {
public:
    AudioFrame(uint8_t * data, int dataSize, bool hardCopy = true) {
        this->dataSize = dataSize;
        this->data = data;
        this->hardCopy = hardCopy;
        if(hardCopy) {
            this->data = static_cast<uint8_t *>(malloc(this->dataSize));
            memcpy(this->data, data, dataSize);
        }
    }

    ~AudioFrame() {
        if(hardCopy && this->data)
            free(this->data);
        this->data = nullptr;
    }

    uint8_t * data = nullptr;
    int dataSize = 0;
    bool hardCopy = true;
};


class AudioRender {
public:
    virtual ~AudioRender()
    {}
    virtual void Init() = 0;
    virtual void ClearAudioCache() = 0;
    virtual void RenderAudioFrame(uint8_t *pData, int dataSize) = 0;
    virtual void UnInit() = 0;

};
#endif //FFMPEGPLAYER_AUDIO_RENDER_H
