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

     external fun ffmpegInfo(): String

     external fun createPlayer(path: String, surface: Surface): Int

     external fun play(player: Int)

     external fun pause(player: Int)


}