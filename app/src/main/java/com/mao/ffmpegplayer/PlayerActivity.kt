package com.mao.ffmpegplayer

import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.mao.ffmpegplayer.databinding.ActivityPlayerBinding
import com.mao.ffplayer.FFMediaPlayer
import android.widget.SeekBar
import com.mao.ffplayer.FFMediaPlayer.Companion.MSG_DECODER_DONE
import com.mao.ffplayer.FFMediaPlayer.Companion.MSG_DECODER_INIT_ERROR
import com.mao.ffplayer.FFMediaPlayer.Companion.MSG_DECODER_READY
import com.mao.ffplayer.FFMediaPlayer.Companion.MSG_DECODING_TIME
import com.mao.ffplayer.FFMediaPlayer.Companion.MSG_REQUEST_RENDER
import com.mao.ffplayer.FFMediaPlayer.Companion.VIDEO_RENDER_ANWINDOW
import android.os.Build
import android.os.Environment
import com.mao.ffplayer.FFMediaPlayer.Companion.MEDIA_PARAM_VIDEO_DURATION
import com.mao.ffplayer.FFMediaPlayer.Companion.MEDIA_PARAM_VIDEO_HEIGHT
import com.mao.ffplayer.FFMediaPlayer.Companion.MEDIA_PARAM_VIDEO_WIDTH
import com.mao.ffplayer.surfaceview.NormalSurfaceView
import kotlin.math.log


/**
 * 播放器 Activity
 * @Description: 播放器 Activity
 * @author maoqitian
 * @date 2020/12/7 0007 8:51
 */
class PlayerActivity :AppCompatActivity(),SurfaceHolder.Callback, FFMediaPlayer.EventCallback{

    private  var  mMediaPlayer: FFMediaPlayer?=null
    private lateinit var surfaceView: NormalSurfaceView

    lateinit var playerBinding: ActivityPlayerBinding

    private lateinit var mSeekBar: SeekBar

    private var mIsTouch = false

    //private val mVideoPath = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov"
    private val mVideoPath: String = Environment.getExternalStorageDirectory().absolutePath+ "/video/one_piece.mp4"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        playerBinding = ActivityPlayerBinding.inflate(layoutInflater)
        setContentView(playerBinding.root)

        surfaceView = playerBinding.surfaceView

        surfaceView.holder.addCallback(this)

        mSeekBar = playerBinding.seekBar


        Log.e("maoqitian","${mVideoPath.toString()}")

        mSeekBar.setOnTouchListener(object :View.OnTouchListener{
            override fun onTouch(p0: View?, p1: MotionEvent?): Boolean {
                    return true
            }

        })
        mSeekBar.setOnSeekBarChangeListener(object :SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {
                mIsTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {
                Log.d("maoqitian", "onStopTrackingTouch() called with: progress = [" + seekBar.progress + "]");
                mMediaPlayer?.seekToPosition(mSeekBar.progress.toFloat())
                mIsTouch = false;
            }
        })


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

    override fun surfaceCreated(surfaceHolder: SurfaceHolder) {
        Log.d(
            "maoqittian",
            "surfaceCreated() called with: surfaceHolder = [$surfaceHolder]"
        )
        mMediaPlayer = FFMediaPlayer()
        mMediaPlayer?.addEventCallback(this)
        mMediaPlayer?.init(mVideoPath, VIDEO_RENDER_ANWINDOW, surfaceHolder.getSurface())

    }

    override fun surfaceChanged(surfaceHolder: SurfaceHolder, p1: Int, p2: Int, p3: Int) {
        mMediaPlayer?.play()
    }

    override fun surfaceDestroyed(surfaceHolder: SurfaceHolder) {
        Log.d("maoqitian", "surfaceDestroyed() called with: surfaceHolder = [" + surfaceHolder + "]");
        mMediaPlayer?.unInit()
    }

    override fun onPlayerEvent(msgType: Int, msgValue: Float) {
        Log.d(
            "maoqitian",
            "onPlayerEvent() called with: msgType = [$msgType], msgValue = [$msgValue]"
        )
        runOnUiThread {
            when (msgType) {
                MSG_DECODER_INIT_ERROR -> {
                }
                MSG_DECODER_READY -> onDecoderReady()
                MSG_DECODER_DONE -> {
                }
                MSG_REQUEST_RENDER -> {
                }
                MSG_DECODING_TIME -> if (!mIsTouch) mSeekBar.progress = msgValue.toInt()
                else -> {
                }
            }
        }
    }

    private fun onDecoderReady() {
        val videoWidth = mMediaPlayer!!.getMediaParams(MEDIA_PARAM_VIDEO_WIDTH).toInt()
        val videoHeight = mMediaPlayer!!.getMediaParams(MEDIA_PARAM_VIDEO_HEIGHT).toInt()
        if (videoHeight * videoWidth != 0) surfaceView.setAspectRatio(videoWidth, videoHeight)

        val duration = mMediaPlayer!!.getMediaParams(MEDIA_PARAM_VIDEO_DURATION).toInt()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            mSeekBar.min = 0
        }
        mSeekBar.max = duration
    }


}