//
// Created by maoqitian on 2020/12/15 0015.
// 枚举 解码器解码的状态
//

#ifndef FFMPEGPLAYER_DECODE_STATE_H
#define FFMPEGPLAYER_DECODE_STATE_H
enum DecodeState {
    PREPARE,
    START,
    DECODING,
    PAUSE,
    STOP,
    FINISH
};
#endif //FFMPEGPLAYER_DECODE_STATE_H
