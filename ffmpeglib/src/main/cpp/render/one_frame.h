//
// Created by maoqitian on 2020/12/16 0016.
// 一个数据（音频/视频）帧数据
//

#ifndef FFMPEGPLAYER_ONE_FRAME_H
#define FFMPEGPLAYER_ONE_FRAME_H

#include <malloc.h>

extern "C"{
#include <libavutil/rational.h>
};
class OneFrame{
public:
    uint8_t *data = NULL;
    int line_size;
    int64_t pts;
    AVRational time_base;
    uint8_t *ext_data = NULL;

    // 是否自动回收data和ext_data
    bool autoRecycle = true;
    //构造函数
    OneFrame(uint8_t *data, int line_size, int64_t pts, AVRational time_base,
             uint8_t *ext_data = NULL, bool autoRecycle = true) {
        this->data = data;
        this->line_size = line_size;
        this->pts = pts;
        this->time_base = time_base;
        this->ext_data = ext_data;
        this->autoRecycle = autoRecycle;
    }
    //析构函数
    ~OneFrame() {
        if (autoRecycle) {
            if (data != NULL) {
                free(data);
                data = NULL;
            }
            if (ext_data != NULL) {
                free(ext_data);
                ext_data = NULL;
            }
        }
    }
};
#endif //FFMPEGPLAYER_ONE_FRAME_H
