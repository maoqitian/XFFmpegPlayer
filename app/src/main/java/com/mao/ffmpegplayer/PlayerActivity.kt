package com.mao.ffmpegplayer

import android.Manifest
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.mao.ffplayer.FFMediaPlayer
import com.permissionx.guolindev.PermissionX
import kotlinx.android.synthetic.main.activity_player.*
import java.io.File


/**
 * @Description: 播放器 Activity
 * @author maoqitian
 * @date 2020/12/7 0007 8:51
 */
class PlayerActivity :AppCompatActivity(),View.OnClickListener,FFMediaPlayer.OnPreparedListener,FFMediaPlayer.OnErrorListener{

    private lateinit var  mediaPlayer: FFMediaPlayer

    private lateinit var surfaceHolder: SurfaceHolder
    private lateinit var surfaceView: SurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_player)
        surfaceView = findViewById(R.id.surfaceView)

        initListener()
        initSurfaceViewAndPlayer()
    }

    private fun initSurfaceViewAndPlayer() {
        mediaPlayer = FFMediaPlayer()
        mediaPlayer.setOnPreparedListener(this)
        mediaPlayer.setOnErrorListener(this)

        surfaceHolder = surfaceView.holder
        surfaceHolder.addCallback(object :SurfaceHolder.Callback{
            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int) {}

            override fun surfaceDestroyed(holder: SurfaceHolder) {
            }

            override fun surfaceCreated(holder: SurfaceHolder) {
                surfaceHolder = holder
                mediaPlayer.setDisPlay(surfaceHolder)
                preparePlayer()
            }
        })
    }
    private fun preparePlayer() {

        //   File file = new File(getExternalFilesDir(null), "test.mp4");
        val file = File(Environment.getExternalStorageDirectory().toString() + File.separator + "190204084208765161.mp4")
        val path = file.absolutePath
        Log.e("毛麒添","path：$path")
        mediaPlayer.setDataSource(path)
        mediaPlayer.prepareAsync()
    }

    private fun initListener() {
        start.setOnClickListener(this)
        stop.setOnClickListener(this)
        pause.setOnClickListener(this)
        reset.setOnClickListener(this)
        restart.setOnClickListener(this)
        setDataSource.setOnClickListener(this)
        prepare.setOnClickListener(this)
        release.setOnClickListener(this)
    }

    override fun onDestroy() {
        super.onDestroy()
        mediaPlayer.reset()
        mediaPlayer.release()
    }

    override fun onClick(v: View) {
        when(v.id){
            R.id.start -> mediaPlayer.start()
            R.id.pause -> mediaPlayer.pause()
            R.id.stop -> mediaPlayer.stop()
        }
    }

    override fun onPrepared(mp: FFMediaPlayer?) {
        Log.e("毛麒添", "onPrepared")
        mediaPlayer.start()
    }

    override fun onError(mp: FFMediaPlayer?, what: Int, extra: Int) {
        Log.e("毛麒添", "onError what：$what extra：$extra")
    }
}