//
// Created by llm on 20-3-21.
//

#include <jni.h>
#include <cassert>
#include <string>
#include <unistd.h>
extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <include/libavutil/imgutils.h>
}
#include <memory>
#include <mutex>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <player/player.h>
#include <include/libavcodec/jni.h>
#include "utils/FFLog.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_ffmpegInfo(JNIEnv *env,jobject  /* this */) {
    char info[40000] = {0};
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    return env->NewStringUTF(info);
}


extern "C" JNIEXPORT jint JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_createPlayer(JNIEnv *env,
                                                       jobject  /* this */,
                                                       jstring path,
                                                       jobject surface) {
    Player *player = new Player(env, path, surface);
    return (jint)(size_t) player;
}

extern "C" JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_play(JNIEnv *env,jobject  /* this */,jint player) {
    Player *p = (Player *) player;
    p->play();
}

extern "C" JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_pause(JNIEnv *env,jobject  /* this */,jint player) {
    Player *p = (Player *) player;
    p->pause();
}
