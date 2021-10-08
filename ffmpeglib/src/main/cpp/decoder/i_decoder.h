//
// Created by maoqitian on 2020/12/15 0015.
// 解码器的基础功能 纯虚类，类似 Java 的 interface

#ifndef FFMPEGPLAYER_I_DECODER_H
#define FFMPEGPLAYER_I_DECODER_H

typedef void (*MessageCallback)(void*, int, float);
typedef long (*AVSyncCallback)(void*);


class IDecoder{
public:
    virtual void Start() = 0; //纯虚函数，达到类似Java 接口抽象的效果，函数实际调用以子类实现为准（virtual 虚函数作用）
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual float GetDuration() = 0;
    virtual void SeekToPosition(float position) = 0;
    virtual float GetCurrentPosition() = 0;
    virtual void SetMessageCallback(void* context, MessageCallback callback) = 0;
};
#endif //FFMPEGPLAYER_I_DECODER_H
