package com.mao.ffplayer.constant

/**
 * @Description: 播放器常量类
 * @author maoqitian
 * @date 2020/12/3 0003 16:10
 */
object PlayerConstants {

    /**
     * Do not change these values without updating their counterparts
     * in include/media/mediaplayer.h!
     */
     const val MEDIA_NOP = 0 // interface test message

     const val MEDIA_PREPARED = 1
     const val MEDIA_PLAYBACK_COMPLETE = 2
     const val MEDIA_BUFFERING_UPDATE = 3
     const val MEDIA_SEEK_COMPLETE = 4
     const val MEDIA_SET_VIDEO_SIZE = 5
     const val MEDIA_STARTED = 6
     const val MEDIA_PAUSED = 7
     const val MEDIA_STOPPED = 8
     const val MEDIA_SKIPPED = 9
     const val MEDIA_NOTIFY_TIME = 98
     const val MEDIA_TIMED_TEXT = 99
     const val MEDIA_ERROR = 100
     const val MEDIA_INFO = 200
     const val MEDIA_SUBTITLE_DATA = 201
     const val MEDIA_META_DATA = 202
     const val MEDIA_DRM_INFO = 210
     const val MEDIA_TIME_DISCONTINUITY = 211
     const val MEDIA_AUDIO_ROUTING_CHANGED = 10000
}