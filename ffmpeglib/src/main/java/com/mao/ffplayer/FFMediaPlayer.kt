package com.mao.ffplayer

import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.Log
import android.view.SurfaceHolder
import com.mao.ffplayer.constant.PlayerConstants
import java.lang.ref.WeakReference

/**
 * @Description: ffmpeg 播放器
 * @author maoqitian
 * @date 2020/12/3 0003 16:09
 */
class FFMediaPlayer {

    /*
    *  jni层用static的变量指向这两个变量
    *  jni在创建了native层的player后会将native player设置给mNativeContext，mNativeContext跟实例相关
    *  Android 的MediaPlayer接口有可能同时被多个实例创建， 需要保存对应的实例，而jni层用了static，跟对象无关
    *  后续jni 对native player进行调用时，需要获取到对应的实例对象， 通过mNativeContext获取，即获取到对应的nativie player实例
    * */
    // accessed by native method, java层保存的nativie层 mediaplayer对象
    private var mNativeContext: Long = 0
    // accessed by native methods
    private var mNativeSurfaceTexture: Long = 0


    var mOnPreparedListener: OnPreparedListener? = null
    var mOnCompletionListener: OnCompletionListener? = null
    var mOnErrorListener: OnErrorListener ? = null
    private val mSurfaceHolder: SurfaceHolder? = null
    private val mEventHandler: EventHandler? = null

    companion object {
        init {
            System.loadLibrary("ffplayer")
        }
        //处理 Handler 内存泄漏
        class EventHandler( mp: FFMediaPlayer, looper: Looper) : Handler(looper) {
            private var weakReference: WeakReference<FFMediaPlayer> = WeakReference(mp)

            override fun handleMessage(msg: Message) {
                val mp = weakReference.get()
                Log.w("FFMediaPlayer", "mediaplayer went away with unhandled events")
                when (msg.what) {
                    PlayerConstants.MEDIA_PREPARED ->
                        if (mp != null) {
                            mp.mOnPreparedListener?.onPrepared(mp)
                        }

                    PlayerConstants.MEDIA_PLAYBACK_COMPLETE ->
                        if (mp != null) {
                            mp.mOnCompletionListener?.onCompletion(mp)
                        }
                    else -> { Log.e("", "Unknown message type " + msg.what)
                    }
                }
            }

        }
    }
    init {
        native_init()

    }

    //Jni 层回调的函数
    private fun postEventFromNative(mediaplayer_ref: Any, what: Int, arg1: Int, arg2: Int) {

    }





    interface OnPreparedListener {
        fun onPrepared(mp: FFMediaPlayer?)
    }

    fun setOnPreparedListener(listener: OnPreparedListener) {
        mOnPreparedListener = listener
    }

    interface OnErrorListener {
        fun onError(mp: FFMediaPlayer?, what: Int, extra: Int): Boolean
    }

    fun setOnErrorListener(listener: OnErrorListener) {
        mOnErrorListener = listener
    }

    interface OnCompletionListener {
        fun onCompletion(mp: FFMediaPlayer?)
    }

    fun setOnCompletionListener(listener: OnCompletionListener) {
        mOnCompletionListener = listener
    }

    external fun native_init()
    private external fun native_setup(mediaplayer_this: Any)
    private external fun native_setDataSource(filePath: String)
    private external fun native_testCallback(bNewThread: Boolean)
    private external fun native_prepareAsync()
    private external fun native_setSurface(surface: Any)

    @Throws(IllegalStateException::class)
    private external fun native_start()

    @Throws(IllegalStateException::class)
    private external fun native_stop()

    @Throws(IllegalStateException::class)
    private external fun native_reset()

    @Throws(IllegalStateException::class)
    private external fun native_release()

    @Throws(IllegalStateException::class)
    private external fun native_pause()

}