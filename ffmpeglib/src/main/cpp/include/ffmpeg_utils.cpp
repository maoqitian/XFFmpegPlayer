//
// Created by maoqitian on 2020/11/29.
//

#include <jni.h>
/*#include <libyuv.h>
#include <libyuv/convert_from.h>*/

extern "C" {
#include <libavutil/avutil.h>
/*#include "libavcodec/avcodec.h"
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


JNIEXPORT jstring JNICALL
Java_com_mao_ffmpegplayer_MainActivity_getVersion(JNIEnv *env, jobject clazz) {
    const char *version = av_version_info();
    return env->NewStringUTF(version);
}

}