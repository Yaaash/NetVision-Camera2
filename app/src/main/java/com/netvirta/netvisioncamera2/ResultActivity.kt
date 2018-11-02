package com.netvirta.netvisioncamera2

import android.graphics.Bitmap
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_result.*

class ResultActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_result)

        val bitmap = intent.getParcelableExtra<Bitmap>("bitmap")
        result_iv?.setImageBitmap(bitmap)
    }
}
