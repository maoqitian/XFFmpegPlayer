package com.mao.ffmpegplayer

import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.mao.ffmpegplayer.databinding.ActivityGlplayer2Binding
import com.mao.ffplayer.FFMediaPlayer
import com.mao.ffplayer.FFMediaPlayer.Companion.VIDEO_GL_RENDER
import com.mao.ffplayer.FFMediaPlayer.Companion.VIDEO_RENDER_OPENGL
import com.mao.ffplayer.glsurface.FFGLSurfaceView
import com.mao.ffplayer.glsurface.OnGestureCallback
import okio.buffer
import okio.sink
import okio.source
import java.io.File
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class OpenGLPlayerActivity2 : AppCompatActivity(),GLSurfaceView.Renderer,FFMediaPlayer.EventCallback,OnGestureCallback {

    lateinit var glplayer2Binding: ActivityGlplayer2Binding

    lateinit var mGLSurfaceView: FFGLSurfaceView

    private var mIsTouch = false
    private  var  mMediaPlayer: FFMediaPlayer?=null

    private lateinit var videFile : File


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        glplayer2Binding = ActivityGlplayer2Binding.inflate(layoutInflater)
        setContentView(glplayer2Binding.root)
        videFile = File("$cacheDir/test2.mp4")

        if(!videFile.exists()){
            //使用 okio 复制文件到 缓存文件中
            assets.open("video/test2.mp4").source().use {
                    bufferSource -> videFile.sink().buffer().use {
                it.writeAll(bufferSource)
            }
            }
        }
        iniGLSurface()

        glplayer2Binding.glSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {
                mIsTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {
                Log.d("maoqitian", "onStopTrackingTouch() called with: progress = [" + seekBar.progress + "]");
                mMediaPlayer?.seekToPosition(glplayer2Binding.glSeekBar.progress.toFloat())
                mIsTouch = false
            }
        })
        initFFMediaPlayer()
    }

    private fun initFFMediaPlayer() {
        mMediaPlayer = FFMediaPlayer()
        mMediaPlayer?.addEventCallback(this)
        mMediaPlayer?.init(videFile.absolutePath, VIDEO_RENDER_OPENGL, null)
    }

    private fun iniGLSurface() {
        mGLSurfaceView = glplayer2Binding.glSurfaceView
        mGLSurfaceView.setEGLContextClientVersion(3)
        mGLSurfaceView.setRenderer(this)
        mGLSurfaceView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        mGLSurfaceView.addOnGestureCallback(this);
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        Log.d("maoqitian", "GL onSurfaceCreated")
        FFMediaPlayer.native_OnSurfaceCreated(VIDEO_GL_RENDER)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        Log.d("maoqitian", "GL onSurfaceChanged w：$width, h：$height")

        FFMediaPlayer.native_OnSurfaceChanged(VIDEO_GL_RENDER,width,height)
    }

    override fun onDrawFrame(gl: GL10?) {
        Log.d("maoqitian", "GL onDrawFrame")
        FFMediaPlayer.native_OnDrawFrame(VIDEO_GL_RENDER)
    }


    override fun onResume() {
        super.onResume()
        mMediaPlayer?.play()
    }

    override fun onPause() {
        super.onPause()
        mMediaPlayer?.pause()
    }

    override fun onStop() {
        super.onStop()
        mMediaPlayer?.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        mMediaPlayer?.unInit()
    }

    override fun onPlayerEvent(msgType: Int, msgValue: Float) {
        Log.d(
            "maoqitian",
            "onPlayerEvent() called with: msgType = [$msgType], msgValue = [$msgValue]"
        )
        runOnUiThread {
            when (msgType) {
                FFMediaPlayer.MSG_DECODER_INIT_ERROR -> {
                }
                FFMediaPlayer.MSG_DECODER_READY -> onDecoderReady()
                FFMediaPlayer.MSG_DECODER_DONE -> {
                }
                FFMediaPlayer.MSG_REQUEST_RENDER -> {
                }
                FFMediaPlayer.MSG_DECODING_TIME -> if (!mIsTouch) glplayer2Binding.glSeekBar.progress = msgValue.toInt()
                else -> {
                }
            }
        }
    }

    private fun onDecoderReady() {
        val videoWidth = mMediaPlayer!!.getMediaParams(FFMediaPlayer.MEDIA_PARAM_VIDEO_WIDTH).toInt()
        val videoHeight = mMediaPlayer!!.getMediaParams(FFMediaPlayer.MEDIA_PARAM_VIDEO_HEIGHT).toInt()
        if (videoHeight * videoWidth != 0) mGLSurfaceView.setAspectRatio(videoWidth, videoHeight)

        val duration = mMediaPlayer!!.getMediaParams(FFMediaPlayer.MEDIA_PARAM_VIDEO_DURATION).toInt()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            glplayer2Binding.glSeekBar.min = 0
        }
        glplayer2Binding.glSeekBar.max = duration
    }

    override fun onGesture(xRotateAngle: Int, yRotateAngle: Int, scale: Float) {

    }

    override fun onTouchLoc(touchX: Float, touchY: Float) {
    }
}