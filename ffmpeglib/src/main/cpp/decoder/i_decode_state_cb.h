//
// Created by maoqitian on 2020/12/17 0017.
// Description: 编码回调接口定义
//

#ifndef FFMPEGPLAYER_I_DECODE_STATE_CB_H
#define FFMPEGPLAYER_I_DECODE_STATE_CB_H

#include "../render/one_frame.h"

//声明IDecoder，在cpp中include，编码重复引用
class IDecoder;

class IDecodeStateCb  {
public:
    IDecodeStateCb();
    virtual void DecodePrepare(IDecoder *decoder) = 0;
    virtual void DecodeReady(IDecoder *decoder) = 0;
    virtual void DecodeRunning(IDecoder *decoder) = 0;
    virtual void DecodePause(IDecoder *decoder) = 0;
    virtual bool DecodeOneFrame(IDecoder *decoder, OneFrame *frame) = 0;
    virtual void DecodeFinish(IDecoder *decoder) = 0;
    virtual void DecodeStop(IDecoder *decoder) = 0;
};


#endif //FFMPEGPLAYER_I_DECODE_STATE_CB_H
