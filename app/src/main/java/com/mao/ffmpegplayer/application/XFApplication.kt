package com.mao.ffmpegplayer.application

import android.app.Application
import com.tencent.bugly.crashreport.CrashReport

/**
 * @Description:
 * @author maoqitian
 * @date 2021/8/19 0019 11:15
 */
class XFApplication :Application() {

    override fun onCreate() {
        super.onCreate()
        //CrashReport.initCrashReport(applicationContext, "b0e1710179", true)

    }
}