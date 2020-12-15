//
// Created by maoqitian on 2020/12/15 0015.
// 视频解码器

#include "v_decoder.h"



VideoDecoder::VideoDecoder(JNIEnv *env, jstring path, bool for_synthesizer):BaseDecoder(env,path,for_synthesizer) {

}

//析构函数
VideoDecoder::~VideoDecoder(){

}