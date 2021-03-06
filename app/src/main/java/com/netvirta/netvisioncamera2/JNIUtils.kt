package com.netvirta.netvisioncamera2

import android.content.Context
import android.graphics.Bitmap
import android.graphics.ImageFormat
import android.media.Image
import android.util.Log
import android.view.Surface
import android.widget.Toast
import org.opencv.core.CvType
import org.opencv.core.CvType.CV_8UC4
import org.opencv.core.Mat
import org.opencv.core.Size
import org.opencv.imgproc.Imgproc
import java.nio.ByteBuffer


object JNIUtils {

    init {
        /**
         * Used to load the 'native-lib' library on application startup.
         */
        System.loadLibrary("native-lib")
    }

    /**
     * Takes an Android Image in the YUV_420_888 format and returns an OpenCV Mat.
     *
     * @param image Image in the YUV_420_888 format.
     * @return OpenCV Mat.
     */
    fun imageToMat(image: Image): Mat {
        var buffer: ByteBuffer
        var rowStride: Int
        var pixelStride: Int
        val width = image.width
        val height = image.height
        var offset = 0

        val planes = image.planes
        val data =
            ByteArray(image.width * image.height * 3 / 2)// ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8];
        val rowData = ByteArray(planes[0].rowStride)

        for (i in planes.indices) {
            buffer = planes[i].buffer
            rowStride = planes[i].rowStride
            pixelStride = planes[i].pixelStride
            val w = if (i == 0) width else width / 2
            val h = if (i == 0) height else height / 2
            for (row in 0 until h) {
                val bytesPerPixel = ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8
                if (pixelStride == bytesPerPixel) {
                    val length = w * bytesPerPixel
                    buffer.get(data, offset, length)

                    // Advance buffer the remainder of the row stride, unless on the last row.
                    // Otherwise, this will throw an IllegalArgumentException because the buffer
                    // doesn't include the last padding.
                    if (h - row != 1) {
                        buffer.position(buffer.position() + rowStride - length)
                    }
                    offset += length
                } else {

                    // On the last row only read the width of the image minus the pixel stride
                    // plus one. Otherwise, this will throw a BufferUnderflowException because the
                    // buffer doesn't include the last padding.
                    if (h - row == 1) {
                        buffer.get(rowData, 0, width - pixelStride + 1)
                    } else {
                        buffer.get(rowData, 0, rowStride)
                    }

                    for (col in 0 until w) {
                        data[offset++] = rowData[col * pixelStride]
                    }
                }
            }
        }

        // Finally, create the Mat.
        val mat = Mat(height + height / 2, width, CvType.CV_8UC1)
        mat.put(0, 0, data)

        return mat
    }

    /**
     * Use native code to copy the contents of sourceImage to surface. sourceImage must have format
     * YUV_420_888, surface must be YV12 and have been configured with
     * `configureSurface()`.
     */
    fun detectLine(context : Context?, sourceImage: Image, surface: Surface, path: String): Mat {

        if (sourceImage.format != ImageFormat.YUV_420_888) {
            throw IllegalArgumentException("sourceImage must have format YUV_420_888.")
        }

        val planes = sourceImage.planes
        // Spec guarantees that planes[0] is luma and has pixel stride of 1.
        // It also guarantees that planes[1] and planes[2] have the same row and
        // pixel stride.
        if (planes[1].pixelStride != 1 && planes[1].pixelStride != 2) {
            throw IllegalArgumentException(
                "sourceImage chroma plane must have a pixel stride of 1 or 2: got " + planes[1].pixelStride
            )
        }
        val destinationMat = Mat(sourceImage.height, sourceImage.width, CV_8UC4)
        Log.e("Yashika", sourceImage.width.toString())
        Log.e("Yashika", sourceImage.height.toString())
       val lines =  detectLine(
            sourceImage.width, sourceImage.height, planes[0].buffer, surface, path, destinationMat.nativeObjAddr
        )
        Toast.makeText(context, "Lines detected : $lines", Toast.LENGTH_LONG).show()
        return destinationMat
    }

    private external fun detectLine(
        width: Int, height: Int, imageBuffer: ByteBuffer,
        surface: Surface, filePath: String, destinationMatAddr : Long
    ): String
}