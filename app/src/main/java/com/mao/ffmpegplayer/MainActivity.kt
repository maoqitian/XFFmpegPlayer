package com.mao.ffmpegplayer

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity(),View.OnClickListener{
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        button.setOnClickListener(this)
        button2.setOnClickListener(this)
        button3.setOnClickListener(this)
        button4.setOnClickListener(this)
        button5.setOnClickListener(this)

    }
    override fun onClick(v: View) {
         when(v.id){
                 R.id.button -> tvText.text = getVersion()
                 R.id.button2 -> tvText.text = avformatInfo()
                 R.id.button3 -> tvText.text = avcodecInfo()
                 R.id.button4 -> tvText.text = avfilterInfo()
                 R.id.button5 -> tvText.text = configurationInfo()
         }
    }

    /**
     * A native method that is implemented by the  native library,
     * which is packaged with this application.
     */
    //获取版本 实际获取的是实际的发布版本号或git提交描述
    private external fun getVersion(): String
    private external fun avformatInfo(): String
    private external fun avcodecInfo(): String
    private external fun avfilterInfo(): String
    //获取配置 so 文件编译配置信息
    private external fun configurationInfo(): String

    companion object {
        init {
            System.loadLibrary("utils")
        }
    }


}