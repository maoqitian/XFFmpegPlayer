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
class PlayerActivity :AppCompatActivity(),View.OnClickListener{

    private lateinit var  mediaPlayer: FFMediaPlayer

    private lateinit var surfaceHolder: SurfaceHolder
    private lateinit var surfaceView: SurfaceView

    private var player: Int? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_player)
        surfaceView = findViewById(R.id.surfaceView)
        initListener()
        initSurfaceViewAndPlayer()
        tv.text = mediaPlayer.ffmpegInfo()
    }

    private fun initSurfaceViewAndPlayer() {
        mediaPlayer = FFMediaPlayer()

        val path = Environment.getExternalStorageDirectory().absolutePath+"/1608131634574.mp4"
        //1608131634574.mp4
        //190204084208765161.mp4
        //SVID_20191203_143408_1.mp4
        //"rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov"
        Log.e("毛麒添", "视频文件$path")
        if (File(path).exists()) {
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
                    if (player == null) {
                        player = mediaPlayer.createPlayer(path, holder.surface)
                    }
                }
            })
        }else{
            Toast.makeText(this, "视频文件不存在，请在手机根目录下放置 mvtest.mp4", Toast.LENGTH_SHORT).show()
        }
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

    override fun onClick(v: View) {
        when(v.id){
            R.id.start -> mediaPlayer.play(player!!)
            R.id.pause -> mediaPlayer.pause(player!!)
        }
    }

}