package com.mao.ffplayer

import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import com.mao.ffplayer.constant.PlayerConstants
import java.io.File
import java.io.IOException
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
    private var mSurfaceHolder: SurfaceHolder? = null
    var surface: Surface? = null
    private var mEventHandler: EventHandler? = null

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
                    PlayerConstants.MEDIA_ERROR ->
                        if (mp != null) {
                            mp.mOnErrorListener?.onError(mp,msg.what,msg.arg1)
                        }
                    else -> { Log.e("", "Unknown message type " + msg.what)
                    }
                }
            }

        }
    }

    init {
        native_init()

        mEventHandler = Looper.myLooper()?.let { EventHandler(this, it) }
        mEventHandler = Looper.getMainLooper()?.let { EventHandler(this, it) }

        native_setup(WeakReference<FFMediaPlayer>(this))
    }

    fun setDisPlay(holder: SurfaceHolder?){
        mSurfaceHolder = holder
        if (holder!=null){
            surface = holder.surface
        }
        surface?.let { native_setSurface(it) }
    }

    fun setDataSource( path:String){
        val file = File(path)
        if(file.exists()){
            native_setDataSource(path)
        } else {
            throw IOException("setDataSource failed.")
        }
    }

    fun prepareAsync(){
        native_prepareAsync()
    }
    fun start(){
       native_start()
    }
    fun pause(){
       native_pause()
    }

    fun stop(){
        native_stop()
    }

    fun reset(){
        native_reset()
        mEventHandler?.removeCallbacksAndMessages(null)
    }

    fun release(){
        mOnPreparedListener = null
        mOnCompletionListener = null
        mOnErrorListener = null

        native_release()
    }

    //Jni 层回调的函数
    private fun postEventFromNative(mediaplayer_ref: Any, what: Int, arg1: Int, arg2: Int) {
        val mp: FFMediaPlayer = (mediaplayer_ref as WeakReference<*>).get() as FFMediaPlayer? ?: return

        if (mp.mEventHandler != null) {
            val m: Message = mp.mEventHandler!!.obtainMessage(what, arg1, arg2)
            mp.mEventHandler!!.sendMessage(m)
        }
    }


    interface OnPreparedListener {
        fun onPrepared(mp: FFMediaPlayer?)
    }

    fun setOnPreparedListener(listener: OnPreparedListener) {
        mOnPreparedListener = listener
    }

    interface OnErrorListener {
        fun onError(mp: FFMediaPlayer?, what: Int, extra: Int)
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