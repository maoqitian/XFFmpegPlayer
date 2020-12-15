//
// Created by maoqitian on 2020/12/15 0015.
// 解码器的基础功能 纯虚类，类似 Java 的 interface

#ifndef FFMPEGPLAYER_I_DECODER_H
#define FFMPEGPLAYER_I_DECODER_H
class IDecoder{
public:
    virtual void GoOn() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual bool IsRunning() = 0;
    virtual long GetDuration() = 0;
    virtual long GetCurPos() = 0;
};
#endif //FFMPEGPLAYER_I_DECODER_H
