package com.mao.ffplayer.glsurface

/**
 * @Description: 手势回调 FFGLSurfaceView
 * @author maoqitian
 * @date 2021/11/10 0010 10:19
 */
interface OnGestureCallback {

    fun onGesture(xRotateAngle: Int, yRotateAngle: Int, scale: Float)
    fun onTouchLoc( touchX : Float, touchY : Float)
}