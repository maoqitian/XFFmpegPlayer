package com.mao.ffmpegplayer.application

import android.app.Application
import com.tencent.bugly.crashreport.CrashReport
import tart.AppLaunch
import tart.PreLaunchState

/**
 * @Description:
 * @author maoqitian
 * @date 2021/8/19 0019 11:15
 */
class XFApplication :Application() {

    override fun onCreate() {
        super.onCreate()
        //CrashReport.initCrashReport(applicationContext, "b0e1710179", true)

        AppLaunch.onAppLaunchListeners += { appLaunch ->
            val startType = when (appLaunch.preLaunchState) {
                PreLaunchState.NO_PROCESS -> "cold start"
                PreLaunchState.NO_PROCESS_FIRST_LAUNCH_AFTER_INSTALL -> "cold start"
                PreLaunchState.NO_PROCESS_FIRST_LAUNCH_AFTER_UPGRADE -> "cold start"
                PreLaunchState.PROCESS_WAS_LAUNCHING_IN_BACKGROUND -> "warm start"
                PreLaunchState.NO_ACTIVITY_NO_SAVED_STATE -> "warm start"
                PreLaunchState.NO_ACTIVITY_BUT_SAVED_STATE -> "warm start"
                PreLaunchState.ACTIVITY_WAS_STOPPED -> "hot start"
            }
            val durationMillis = appLaunch.duration.uptimeMillis
            println("$startType launch: $durationMillis ms")
        }

    }
}