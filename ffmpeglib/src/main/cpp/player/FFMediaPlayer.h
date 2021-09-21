//
// Created by maoqitian on 2021/9/20.
// Description: 播放器
//

#ifndef FFMPEGPLAYER_FFMEDIAPLAYER_H
#define FFMPEGPLAYER_FFMEDIAPLAYER_H


#include <jni.h>
#include "../decoder/video/v_decoder.h"
#include "../render/video/video_render.h"
#include "audio_render.h"
#include "a_decoder.h"


#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "playerEventCallback"

#define MEDIA_PARAM_VIDEO_WIDTH         0x0001
#define MEDIA_PARAM_VIDEO_HEIGHT        0x0002
#define MEDIA_PARAM_VIDEO_DURATION      0x0003

class FFMediaPlayer {
public:
    FFMediaPlayer(){};
    ~FFMediaPlayer(){};

    void Init(JNIEnv *jniEnv, jobject obj, char *url, int renderType, jobject surface);
    void UnInit();

    void Play();
    void Pause();
    void Stop();
    void SeekToPosition(float position);
    long GetMediaParams(int paramType);

private:
    JNIEnv *GetJNIEnv(bool *isAttach);
    jobject GetJavaObj();
    JavaVM *GetJavaVM();

    static void PostMessage(void *context, int msgType, float msgCode);

    JavaVM *m_JavaVM = nullptr;
    jobject m_JavaObj = nullptr;

    VideoDecoder *m_VideoDecoder = nullptr;
    AudioDecoder *m_AudioDecoder = nullptr;

    VideoRender *m_VideoRender = nullptr;
    AudioRender *m_AudioRender = nullptr;

};

#endif //FFMPEGPLAYER_FFMEDIAPLAYER_H
