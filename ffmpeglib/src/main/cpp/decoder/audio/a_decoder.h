//
// Created by maoqitian on 2020/12/17 0017.
// Description: 音频解码器
//

#ifndef FFMPEGPLAYER_A_DECODER_H
#define FFMPEGPLAYER_A_DECODER_H


#include <utils/const.h>
#include <render/audio/audio_render.h>
#include "../base_decoder.h"
#include <render/one_frame.h>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
};

class AudioDecoder : public BaseDecoder {

    const char *TAG = "AudioDecoder";

    // 音频转换器
    SwrContext *m_swr = NULL;

    // OpeSL ES 音频渲染播放器
    AudioRender *m_render = NULL;

    // 输出缓冲
    uint8_t *m_out_buffer[2] = {NULL,NULL};

    // 重采样后，每个通道包含的采样数
    // acc默认为1024，重采样后可能会变化
    int m_dest_nb_sample = 1024;

    // 重采样以后，一帧数据的大小
    size_t m_dest_data_size = 0;

    // 初始化转换工具
    void InitSwr();

    // 初始化输出缓冲
    void InitOutBuffer();

    /**
     * 计算重采样后通道采样数和帧数据大小
     */
    void CalculateSampleArgs();

    // 初始化渲染器
    void InitRender();

    // 释放缓冲区
    void ReleaseOutBuffer();

    // 采样格式：16位
    AVSampleFormat GetSampleFmt() {
        if (ForSynthesizer()) {
            return ENCODE_AUDIO_DEST_FORMAT;
        } else{
            return AV_SAMPLE_FMT_S16;
        }

    }

    /**
      * 采样率
      */
    int GetSampleRate(int spr) {
        if (ForSynthesizer()) {
            return ENCODE_AUDIO_DEST_SAMPLE_RATE;
        } else {
            return spr;
        }
    }


public:
    //构造函数
    AudioDecoder(JNIEnv *env, const jstring path, bool forSynthesizer);
    //虚函数
    ~AudioDecoder();

    void SetRender(AudioRender *render);

protected:
    void Prepare(JNIEnv *env) override;
    void Render(AVFrame *frame) override;
    void Release() override;

    bool NeedLoopDecode() override {
        return true;
    }

    AVMediaType GetMediaType() override {
        return AVMEDIA_TYPE_AUDIO;
    }

    const char *const LogSpec() override {
        return "AUDIO";
    }
};


#endif //FFMPEGPLAYER_A_DECODER_H
