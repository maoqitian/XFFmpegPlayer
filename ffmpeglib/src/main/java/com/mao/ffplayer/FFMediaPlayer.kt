package com.mao.ffplayer
import android.view.Surface

/**
 * @Description: ffmpeg 播放器
 * @author maoqitian
 * @date 2020/12/3 0003 16:09
 */
class FFMediaPlayer{

    companion object {

        //ffmepg 各个库版本信息
        external fun native_GetFFmpegVersion(): String

        init {
            System.loadLibrary("ffplayer")
        }

        fun GetFFmpegVersion(): String {
            return native_GetFFmpegVersion()
        }

        //gl render type
        const val VIDEO_GL_RENDER = 0
        const val AUDIO_GL_RENDER = 1
        const val VR_3D_GL_RENDER = 2


        const val MSG_DECODER_INIT_ERROR = 0
        const val MSG_DECODER_READY = 1
        const val MSG_DECODER_DONE = 2
        const val MSG_REQUEST_RENDER = 3
        const val MSG_DECODING_TIME = 4

        const val MEDIA_PARAM_VIDEO_WIDTH = 0x0001
        const val MEDIA_PARAM_VIDEO_HEIGHT = 0x0002
        const val MEDIA_PARAM_VIDEO_DURATION = 0x0003

        //视频渲染类型
        const val VIDEO_RENDER_OPENGL = 0
        const val VIDEO_RENDER_ANWINDOW = 1
        const val VIDEO_RENDER_3D_VR = 2

        //for Open GL render
        external fun native_OnSurfaceCreated(renderType: Int)
        external fun native_OnSurfaceChanged(renderType: Int, width: Int, height: Int)
        external fun native_OnDrawFrame(renderType: Int)


    }

    private var mNativePlayerHandle: Long = 0

    private var mEventCallback: EventCallback? = null



    fun init(url: String?, videoRenderType: Int, surface: Surface?) {
        mNativePlayerHandle = native_Init(url!!, videoRenderType, surface)
    }

    fun play() {
        native_Play(mNativePlayerHandle)
    }

    fun pause() {
        native_Pause(mNativePlayerHandle)
    }

    fun seekToPosition(position: Float) {
        native_SeekToPosition(mNativePlayerHandle, position)
    }

    fun stop() {
        native_Stop(mNativePlayerHandle)
    }

    fun unInit() {
        native_Release(mNativePlayerHandle)
    }

    fun addEventCallback(callback: EventCallback?) {
        mEventCallback = callback
    }

    fun getMediaParams(paramType: Int): Long {
        return native_GetMediaParams(mNativePlayerHandle, paramType)
    }

    private fun playerEventCallback(msgType: Int, msgValue: Float) {
        mEventCallback?.onPlayerEvent(msgType, msgValue)
    }





    //播放器初始化
     external fun native_Init(url: String, renderType: Int, surface: Any?): Long
    //开始播放
     external fun native_Play(playerHandle: Long)
    //指定位置播放
     external fun native_SeekToPosition(playerHandle: Long, position: Float)
    //暂停
     external fun native_Pause(playerHandle: Long)
    //停止
     external fun native_Stop(playerHandle: Long)

     external fun native_Release(playerHandle: Long)

     external fun native_GetMediaParams(playerHandle: Long, paramType: Int): Long

    //for Open GL render
    external fun native_OnSurfaceCreated(renderType: Int)
    external fun native_OnSurfaceChanged(renderType: Int, width: Int, height: Int)
    external fun native_OnDrawFrame(renderType: Int)

    //update MVP matrix
    external fun native_SetGesture(
        renderType: Int,
        xRotateAngle: Float,
        yRotateAngle: Float,
        scale: Float
    )

    external fun native_SetTouchLoc(renderType: Int, touchX: Float, touchY: Float)


    interface EventCallback {
        fun onPlayerEvent(msgType: Int, msgValue: Float)
    }
}