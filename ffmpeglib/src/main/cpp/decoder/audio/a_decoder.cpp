//
// Created by maoqitian on 2020/12/17 0017.
// Description: 音频解码
//

#include "a_decoder.h"
#include <utils/const.h>


void AudioDecoder::OnDecoderReady() {
    LOGCATE("AudioDecoder::OnDecoderReady");

    if (m_AudioRender){
        AVCodecContext *codecContext = GetCodecContext();

        m_SwrContext = swr_alloc();

        av_opt_set_int(m_SwrContext, "in_channel_layout", codecContext->channel_layout, 0);
        av_opt_set_int(m_SwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

        av_opt_set_int(m_SwrContext, "in_sample_rate", codecContext->sample_rate, 0);
        av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

        av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", codecContext->sample_fmt, 0);
        av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", DST_SAMPLT_FORMAT,  0);

        swr_init(m_SwrContext);

        LOGCATE("AudioDecoder::OnDecoderReady audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lld",
                codecContext->sample_rate, codecContext->channels, codecContext->sample_fmt, codecContext->frame_size,codecContext->channel_layout);

        // resample params
        m_nbSamples = (int)av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE, codecContext->sample_rate, AV_ROUND_UP);
        m_DstFrameDataSze = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,m_nbSamples, DST_SAMPLT_FORMAT, 1);

        LOGCATE("AudioDecoder::OnDecoderReady [m_nbSamples, m_DstFrameDataSze]=[%d, %d]", m_nbSamples, m_DstFrameDataSze);

        m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSze);

        m_AudioRender->Init();

    } else{
        LOGCATE("AudioDecoder::OnDecoderReady m_AudioRender == null");

    }

}

//资源释放
void AudioDecoder::OnDecoderDone() {
    LOGCATE("AudioDecoder::OnDecoderDone");
    if(m_AudioRender)
        m_AudioRender->UnInit();

    if(m_AudioOutBuffer) {
        free(m_AudioOutBuffer);
        m_AudioOutBuffer = nullptr;
    }

    if(m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
}

void AudioDecoder::OnFrameAvailable(AVFrame *frame) {
    LOGCATE("AudioDecoder::OnFrameAvailable frame=%p, frame->nb_samples=%d", frame, frame->nb_samples);
    if(m_AudioRender){
        int result = swr_convert(m_SwrContext, &m_AudioOutBuffer,m_DstFrameDataSze/2,(const uint8_t **) frame->data, frame->nb_samples);

        if (result > 0){
            m_AudioRender -> RenderAudioFrame(m_AudioOutBuffer,m_DstFrameDataSze);
        }
    }
}

void AudioDecoder::ClearCache() {
    if(m_AudioRender)
        m_AudioRender->ClearAudioCache();
}

long AudioDecoder::GetAudioDecoderTimestampForAVSync(void *context) {
    if (context != nullptr){
        AudioDecoder* audioDecoder = static_cast<AudioDecoder *>(context);
        return audioDecoder-> GetCurrentPosition();
    }
    return 0;
}
