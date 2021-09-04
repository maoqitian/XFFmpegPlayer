package com.mao.ffplayer
import android.view.Surface

/**
 * @Description: ffmpeg 播放器
 * @author maoqitian
 * @date 2020/12/3 0003 16:09
 */
class FFMediaPlayer {


    companion object {
        init {
            System.loadLibrary("ffplayer")
        }
    }

     // SurfaceView 渲染
     external fun ffmpegInfo(): String

     external fun createPlayer(path: String, surface: Surface): Int

     external fun play(player: Int)

     external fun pause(player: Int)

     //OpenGL ES 视频渲染
     external fun createOpenGLPlayer(path: String, surface: Surface): Int
     //播放或者暂停
     external fun playOrPauseOpenGL(player: Int)
     //停止播放
     external fun stopPlayOpenGL(player: Int)
}