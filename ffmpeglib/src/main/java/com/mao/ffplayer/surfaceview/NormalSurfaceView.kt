package com.mao.ffplayer.surfaceview

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceView
import android.view.View.MeasureSpec

/**
 *  author : maoqitian
 *  date : 2021/9/21 11:12
 *  description :
 */
class NormalSurfaceView : SurfaceView {


    private var mRatioWidth = 0
    private var mRatioHeight = 0

    constructor(context: Context?) : this(context,null)

    constructor (context: Context?, attrs: AttributeSet?) : super(context,attrs)


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

}