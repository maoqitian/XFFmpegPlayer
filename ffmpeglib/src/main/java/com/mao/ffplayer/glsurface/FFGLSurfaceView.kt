package com.mao.ffplayer.glsurface

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.ScaleGestureDetector

/**
 * @Description: OpenGL 视频渲染 支持手势
 * @author maoqitian
 * @date 2021/10/11 0011 10:08
 */
class FFGLSurfaceView constructor(context: Context?, attrs: AttributeSet?):GLSurfaceView(context, attrs),
    ScaleGestureDetector.OnScaleGestureListener{

    var scaleGestureDetector:ScaleGestureDetector = ScaleGestureDetector(context,this)

    private var mRatioWidth = 0
    private var mRatioHeight = 0

    private var mGestureCallback:OnGestureCallback? = null

    //次级构造函数
    constructor(context: Context?) : this(context,null)

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        val width = MeasureSpec.getSize(widthMeasureSpec)
        val height = MeasureSpec.getSize(heightMeasureSpec)

        if (0 == mRatioWidth || 0 == mRatioHeight) {
            setMeasuredDimension(width, height)
        } else {
            if (width < height * mRatioWidth / mRatioHeight) {
                setMeasuredDimension(width, width * mRatioHeight / mRatioWidth)
            } else {
                setMeasuredDimension(height * mRatioWidth / mRatioHeight, height)
            }
        }
    }

    fun setAspectRatio(width: Int, height: Int) {
        //Log.d(TAG, "setAspectRatio() called with: width = [$width], height = [$height]")
        require(!(width < 0 || height < 0)) { "Size cannot be negative." }
        mRatioWidth = width
        mRatioHeight = height
        requestLayout()
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        return super.onTouchEvent(event)
    }

    fun addOnGestureCallback(mListener:OnGestureCallback){
        mGestureCallback = mListener
    }

    override fun onScale(detector: ScaleGestureDetector?): Boolean {
        return false
    }

    override fun onScaleBegin(detector: ScaleGestureDetector?): Boolean {
        return true
    }

    override fun onScaleEnd(detector: ScaleGestureDetector?) {
    }
}