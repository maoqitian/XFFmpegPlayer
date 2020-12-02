//
// Created by maoqitian on 2020/11/29.
//

#include <jni.h>
#include <string>

/*#include <libyuv.h>
#include <libyuv/convert_from.h>*/

extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/avutil.h>
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
/*
#include "android/log.h"
#include "libavformat/avformat.h"
#include "libavutil/error.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include "android/bitmap.h"
#include "libyuv.h"
#include "libyuv/convert_argb.h"
#include "ffmpeg.h"*/

#define logDebug(...) __android_log_print(ANDROID_LOG_DEBUG,"MainActivity",__VA_ARGS__)

//获取版本 实际获取的是实际的发布版本号或git提交描述
JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_getVersion(JNIEnv *env, jobject clazz) {
    const char *version = av_version_info();
    return env->NewStringUTF(version);
}

JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_avformatInfo(JNIEnv *env, jobject instance) {

    char info[40000] = { 0 };

    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    //Input
    while (if_temp != NULL){
        sprintf(info, "%s[In ][%10s]\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    //Output
    while (of_temp != NULL) {
        sprintf(info, "%s[Out][%10s]\n", info, of_temp->name);
        of_temp = of_temp->next;
    }

    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_avcodecInfo(JNIEnv *env, jobject instance) {

    char info[40000] = { 0 };

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp!=NULL) {
        if (c_temp->decode!=NULL){
            sprintf(info, "%s[Dec]", info);
        }
        else{
            sprintf(info, "%s[Enc]", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);


        c_temp=c_temp->next;
    }

    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_avfilterInfo(JNIEnv *env, jobject instance) {

    char info[40000] = { 0 };
    AVFilter *f_temp = (AVFilter *)avfilter_next(NULL);
    while (f_temp != NULL){
        sprintf(info, "%s[%10s]\n", info, f_temp->name);
        f_temp = f_temp->next;
    }
    return env->NewStringUTF(info);
}

//获取配置 so 文件编译配置信息
extern "C" JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_configurationInfo(JNIEnv *env, jobject instance) {

    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

}