package com.netvirta.netvisioncamera2

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.AttributeSet
import android.util.Log
import android.view.MotionEvent
import android.widget.TextView
import android.widget.Toast
import org.opencv.android.CameraGLSurfaceView

/**
 * Custom Surface View for OpenCL. Capture frames and send it to processing in C++/OpenCV
 */
class MyGLSurfaceView(context: Context, attrs: AttributeSet) : CameraGLSurfaceView(context, attrs),
    CameraGLSurfaceView.CameraTextureListener {

    /**
     * Keep frame counts for Camera2
     */
    private var frameCounter: Int = 0
    /**
     * Last Frame Nano time
     */
    private var lastNanoTime: Long = 0
    /**
     * Weather it is Front camera or Rear
     */
    private var frontFacingVar = false
    /**
     * Show Frames per second in Activity
     */
    private var mFpsText: TextView? = null

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(e: MotionEvent): Boolean {
        if (e.action == MotionEvent.ACTION_DOWN)
            (context as Activity).openOptionsMenu()
        return true
    }

    override fun onCameraViewStarted(width: Int, height: Int) {
        frameCounter = 0
        lastNanoTime = System.nanoTime()
    }

    override fun onCameraViewStopped() {
    }

    fun setFrontFacing(frontFacing: Boolean) {
        this.frontFacingVar = frontFacing
    }

    @SuppressLint("SetTextI18n")
    override fun onCameraTexture(texIn: Int, texOut: Int, width: Int, height: Int): Boolean {
        // FPS
        frameCounter++
        if (frameCounter >= 30) {
            val fps = (frameCounter * 1e9 / (System.nanoTime() - lastNanoTime)).toInt()
            Log.i(javaClass.simpleName, "drawFrame() FPS: $fps")
            if (mFpsText != null) {
                val fpsUpdater = Runnable { mFpsText?.text = "FPS: $fps" }
                Handler(Looper.getMainLooper()).post(fpsUpdater)
            } else {
                Log.d(javaClass.simpleName, "mFpsText == null")
                mFpsText = (context as Activity).findViewById(R.id.fps_text_view) as TextView
            }
            frameCounter = 0
            lastNanoTime = System.nanoTime()
        }

        processFrame(texOut, width, height, frontFacingVar)
        return true
    }

    private external fun processFrame(textOutput: Int, w: Int, h: Int, frontFacing: Boolean)

}
