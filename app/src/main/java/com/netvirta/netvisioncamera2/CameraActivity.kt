package com.netvirta.netvisioncamera2

import android.app.Activity
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.view.WindowManager
import kotlinx.android.synthetic.main.activity_camera.*
import org.opencv.android.CameraBridgeViewBase

/**
 * Show [MyGLSurfaceView] and preview visual aids from C++ Processing
 *
 * @author Yashika
 */
class CameraActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
        window.setFlags(
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        )
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE

        setContentView(R.layout.activity_camera)
        setListeners()
    }

    private fun setListeners() {
        /**
         * Set Camera Change listener
         */
        camera_switch?.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                gl_surface_view?.setFrontFacing(true)
                gl_surface_view?.setCameraIndex(CameraBridgeViewBase.CAMERA_ID_FRONT)
            } else {
                gl_surface_view?.setFrontFacing(false)
                gl_surface_view?.setCameraIndex(CameraBridgeViewBase.CAMERA_ID_BACK)
            }
        }

        /**
         * Set maximum preview size for Surface View
         */
        gl_surface_view?.setMaxCameraPreviewSize(1280, 920)
        gl_surface_view?.cameraTextureListener = gl_surface_view
    }

    override fun onResume() {
        /**
         * Resume Surface View
         */
        gl_surface_view?.onResume()
        super.onResume()
    }


    public override fun onPause() {
        /**
         * Pause Surface View
         */
        gl_surface_view?.onPause()
        super.onPause()
    }

    companion object {
        init {
            /**
             * Load Libraries using Cmake
             */
            System.loadLibrary("native-lib")
        }
    }
}
