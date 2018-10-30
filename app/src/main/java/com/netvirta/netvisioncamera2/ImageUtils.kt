package com.netvirta.netvisioncamera2

import android.graphics.ImageFormat
import android.media.Image
import org.opencv.core.CvType
import org.opencv.core.Mat
import java.nio.ByteBuffer


object ImageUtils {

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
        val width = image.getWidth()
        val height = image.getHeight()
        var offset = 0

        val planes = image.getPlanes()
        val data =
            ByteArray(image.getWidth() * image.getHeight() * ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8)
        val rowData = ByteArray(planes[0].getRowStride())

        for (i in planes.indices) {
            buffer = planes[i].getBuffer()
            rowStride = planes[i].getRowStride()
            pixelStride = planes[i].getPixelStride()
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
}