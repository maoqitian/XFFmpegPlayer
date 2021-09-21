//
// Created by maoqitian on 2020/12/17 0017.
// Description: 音频解码器
//

#ifndef FFMPEGPLAYER_A_DECODER_H
#define FFMPEGPLAYER_A_DECODER_H

#include <render/audio/audio_render.h>
#include "../base_decoder.h"
#include "../i_decoder.h"
#include <render/one_frame.h>

extern "C" {
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
}


// 音频编码采样率
static const int AUDIO_DST_SAMPLE_RATE = 44100;
// 音频编码通道数
static const int AUDIO_DST_CHANNEL_COUNTS = 2;
// 音频编码声道格式
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
// 音频编码比特率
static const int AUDIO_DST_BIT_RATE = 64000;
// ACC音频一帧采样数
static const int ACC_NB_SAMPLES = 1024;

class AudioDecoder : public BaseDecoder {

public:
    AudioDecoder(char *url){
        Init(url, AVMEDIA_TYPE_AUDIO);
    }

    virtual ~AudioDecoder(){
        UnInit();
    }

    void SetAudioRender(AudioRender *audioRender)
    {
        m_AudioRender = audioRender;
    }

    static long GetAudioDecoderTimestampForAVSync(void* context);

private:
    virtual void OnDecoderReady();
    virtual void OnDecoderDone();
    virtual void OnFrameAvailable(AVFrame *frame);
    virtual void ClearCache();

    const AVSampleFormat DST_SAMPLT_FORMAT = AV_SAMPLE_FMT_S16;

    AudioRender  *m_AudioRender = nullptr;

    //audio resample context
    SwrContext   *m_SwrContext = nullptr;

    uint8_t      *m_AudioOutBuffer = nullptr;

    //number of sample per channel
    int           m_nbSamples = 0;

    //dst frame data size
    int           m_DstFrameDataSze = 0;
};


#endif //FFMPEGPLAYER_A_DECODER_H
