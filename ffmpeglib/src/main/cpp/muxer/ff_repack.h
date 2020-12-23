//
// Created by maoqitian on 2020/12/18 0018.
// Description: OpenGl 打包器
//

#ifndef FFMPEGPLAYER_FF_REPACK_H
#define FFMPEGPLAYER_FF_REPACK_H

#include <jni.h>
extern "C" {
#include <libavformat/avformat.h>
};

class FFRepack {
private:
    const char *TAG = "FFRepack";

    AVFormatContext *m_in_format_cxt = NULL;

    AVFormatContext *m_out_format_cxt = NULL;

    int OpenSrcFile(const char *srcPath);

    int InitMuxerParams(const char *destPath);

public:
    FFRepack(JNIEnv *env,jstring in_path, jstring out_path);

    void Start();

    void Write(AVPacket pkt);

    void Release();
};


#endif //FFMPEGPLAYER_FF_REPACK_H
