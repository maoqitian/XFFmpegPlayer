//
// Created by maoqitian on 2021/9/20.
// Description: FFMediaPlayer 播放器 对外暴露方法
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
#include <include/libavcodec/jni.h>
#include <AudioGLRender.h>
#include "utils/FFLog.h"
#include "FFMediaPlayer.h"
#include "BaseGLRender.h"



extern "C"
JNIEXPORT jlong JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1Init(JNIEnv *env, jobject thiz, jstring jurl,
                                                 jint render_type, jobject surface) {

    const char* url = env->GetStringUTFChars(jurl, nullptr);
    FFMediaPlayer *player = new FFMediaPlayer();
    player->Init(env,thiz,const_cast<char *>(url),render_type,surface);
    env->ReleaseStringUTFChars(jurl,url);
    return reinterpret_cast<jlong>(player);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1Play(JNIEnv *env, jobject thiz, jlong player_handle) {
    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        ffMediaPlayer->Play();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1SeekToPosition(JNIEnv *env, jobject thiz,
                                                           jlong player_handle, jfloat position) {
    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        ffMediaPlayer->SeekToPosition(position);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1Pause(JNIEnv *env, jobject thiz, jlong player_handle) {
    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        ffMediaPlayer->Pause();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1Stop(JNIEnv *env, jobject thiz, jlong player_handle) {
    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        ffMediaPlayer->Stop();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1Release(JNIEnv *env, jobject thiz,
                                                    jlong player_handle) {

    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        ffMediaPlayer->UnInit();
    }
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1GetMediaParams(JNIEnv *env, jobject thiz,
                                                           jlong player_handle, jint param_type) {

    long value = 0;
    if (player_handle != 0){
        FFMediaPlayer *ffMediaPlayer = reinterpret_cast<FFMediaPlayer *>(player_handle);
        value = ffMediaPlayer->GetMediaParams(param_type);
    }

    return value;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1OnSurfaceCreated(JNIEnv *env, jobject thiz,
                                                             jint render_type) {
    switch (render_type)
    {
        case VIDEO_GL_RENDER:
            //VideoGLRender::GetInstance()->OnSurfaceCreated();
            break;
        case AUDIO_GL_RENDER:
            AudioGLRender::GetInstance()->OnSurfaceCreated();
            break;
            //case VR_3D_GL_RENDER:
            //VRGLRender::GetInstance()->OnSurfaceCreated();
            //break;
        default:
            break;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1OnSurfaceChanged(JNIEnv *env, jobject thiz,
                                                             jint render_type, jint width,
                                                             jint height) {
    switch (render_type)
    {
        case VIDEO_GL_RENDER:
            //VideoGLRender::GetInstance()->OnSurfaceCreated();
            break;
        case AUDIO_GL_RENDER:
            AudioGLRender::GetInstance()->OnSurfaceChanged(width,height);
            break;
        case VR_3D_GL_RENDER:
            //VRGLRender::GetInstance()->OnSurfaceCreated();
            break;
        default:
            break;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1OnDrawFrame(JNIEnv *env, jobject thiz,
                                                        jint render_type) {
    switch (render_type)
    {
        case VIDEO_GL_RENDER:
            //VideoGLRender::GetInstance()->OnSurfaceCreated();
            break;
        case AUDIO_GL_RENDER:
            AudioGLRender::GetInstance()->OnDrawFrame();
            break;
            case VR_3D_GL_RENDER:
            //VRGLRender::GetInstance()->OnSurfaceCreated();
            break;
        default:
            break;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1SetGesture(JNIEnv *env, jobject thiz, jint render_type,
                                                       jfloat x_rotate_angle, jfloat y_rotate_angle,
                                                       jfloat scale) {
    switch (render_type)
    {
        case VIDEO_GL_RENDER:
            //VideoGLRender::GetInstance()->UpdateMVPMatrix(x_rotate_angle, y_rotate_angle, scale, scale);
            break;
        case AUDIO_GL_RENDER:
            AudioGLRender::GetInstance()->UpdateMVPMatrix(x_rotate_angle, y_rotate_angle, scale, scale);
            break;
        case VR_3D_GL_RENDER:
            //VRGLRender::GetInstance()->UpdateMVPMatrix(x_rotate_angle, y_rotate_angle, scale, scale);
            break;
        default:
            break;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_native_1SetTouchLoc(JNIEnv *env, jobject thiz, jint render_type,
                                                        jfloat touch_x, jfloat touch_y) {
    switch (render_type)
    {
        case VIDEO_GL_RENDER:
            //VideoGLRender::GetInstance()->SetTouchLoc(touch_x, touch_y);
            break;
        case AUDIO_GL_RENDER:
            AudioGLRender::GetInstance()->SetTouchLoc(touch_x, touch_y);
            break;
        case VR_3D_GL_RENDER:
            //VRGLRender::GetInstance()->SetTouchLoc(touch_x, touch_y);
            break;
        default:
            break;
    }
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_mao_ffplayer_FFMediaPlayer_00024Companion_native_1GetFFmpegVersion(JNIEnv *env,
                                                                            jobject thiz) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_configure : \n");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
    LOGCATE("GetFFmpegVersion\n%s", strBuffer);

    //ASanTestCase::MainTest();

    return env->NewStringUTF(strBuffer);
}