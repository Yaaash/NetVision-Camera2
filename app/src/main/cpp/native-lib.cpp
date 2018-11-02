#include <jni.h>
#include <opencv2/opencv.hpp>
#include <android/native_window_jni.h>

#include <string>
#include <vector>
#include <android/bitmap.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

using namespace cv;
using namespace std;


#ifndef int64_t
#define int64_t long long
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int32_t
#define int32_t int
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

extern "C" JNIEXPORT jstring
JNICALL
Java_com_netvirta_netvisioncamera2_JNIUtils_detectLine(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight,
        jobject srcBuffer, jobject dstSurface) {

    char outStr[200];

    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));

    if (srcLumaPtr == nullptr) {
        return NULL;
    }

    int dstWidth;
    int dstHeight;

    cv::Mat mYuv(srcHeight + srcHeight / 2, srcWidth, CV_8UC1, srcLumaPtr);
//
    ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;

    dstWidth = srcHeight;
    dstHeight = srcWidth;

    ANativeWindow_setBuffersGeometry(win, dstWidth, dstHeight, 0 /*format unchanged*/);

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
//        LOGE("ANativeWindow_lock failed with error code %d\n", err);
        ANativeWindow_release(win);
        return NULL;
    }
//
    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4,
                dstLumaPtr);        // TextureView buffer, use stride as width
    Mat srcRgba(srcHeight, srcWidth, CV_8UC4);
    Mat flipRgba(dstHeight, dstWidth, CV_8UC4);
//
//    // convert YUV -> RGBA
    cv::cvtColor(mYuv, srcRgba, CV_YUV2RGBA_NV21);
//
//    // Rotate 90 degree
    cv::transpose(srcRgba, flipRgba);
    cv::flip(flipRgba, flipRgba, 1);
//
    Mat destination;

    // Detect the edges of the image by using a Canny detector
    Canny(flipRgba, destination, 50, 200, 3);

    vector<Vec4i> lines;
    HoughLinesP(destination, lines, 1, CV_PI / 180, 50, 50, 10);

    uchar *dbuf;
    uchar *sbuf;
    dbuf = destination.data;
    sbuf = flipRgba.data;

    int i;
    for (i = 0; i < flipRgba.rows; i++) {
        dbuf = dstRgba.data + i * buf.stride * 4;
        memcpy(dbuf, sbuf, flipRgba.cols * 4);
        sbuf += flipRgba.cols * 4;
    }
    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i l = lines[i];
        //  display the result by drawing the lines.
        cv::line(destination, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
    }

    // write back
    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);

    return env->NewStringUTF(outStr);
}


//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Camera2Demo", __VA_ARGS__)

//convert Y Plane from YUV_420_888 to RGBA and display
extern "C" {
JNIEXPORT void JNICALL Java_com_netvirta_netvisioncamera2_JNIUtils_GrayscaleDisplay(
        JNIEnv *env,
        jobject obj,
        jint srcWidth,
        jint srcHeight,
        jint rowStride,
        jobject srcBuffer,
        jobject surface) {

    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));
    /*
    if (srcLumaPtr == nullptr) {
        LOGE("srcLumaPtr null ERROR!");
        return NULL;
    }
    */

    ANativeWindow * window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_acquire(window);
    ANativeWindow_Buffer buffer;
    //set output size and format
    //only 3 formats are available:
    //WINDOW_FORMAT_RGBA_8888(DEFAULT), WINDOW_FORMAT_RGBX_8888, WINDOW_FORMAT_RGB_565
    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);
    if (int32_t err = ANativeWindow_lock(window, &buffer, NULL)) {
//        LOGE("ANativeWindow_lock failed with error code: %d\n", err);
        ANativeWindow_release(window);
    }

    //to display grayscale, first convert the Y plane from YUV_420_888 to RGBA
    //ANativeWindow_Buffer buffer;
    uint8_t * outPtr = reinterpret_cast<uint8_t *>(buffer.bits);
    for (size_t y = 0; y < srcHeight; y++)
    {
        uint8_t * rowPtr = srcLumaPtr + y * rowStride;
        for (size_t x = 0; x < srcWidth; x++)
        {
            //for grayscale output, just duplicate the Y channel into R, G, B channels
            *(outPtr++) = *rowPtr; //R
            *(outPtr++) = *rowPtr; //G
            *(outPtr++) = *rowPtr; //B
            *(outPtr++) = 255; // gamma for RGBA_8888
            ++rowPtr;
        }
    }

    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);
}

//do YUV_420_88 to RGBA_8888 conversion, flip, and display
JNIEXPORT void JNICALL Java_com_netvirta_netvisioncamera2_JNIUtils_RGBADisplay(
        JNIEnv *env,
        jobject obj,
        jint srcWidth,
        jint srcHeight,
        jint Y_rowStride,
        jobject Y_Buffer,
        jint UV_rowStride,
        jobject U_Buffer,
        jobject V_Buffer,
        jobject surface) {

    uint8_t *srcYPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(Y_Buffer));
    uint8_t *srcUPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(U_Buffer));
    uint8_t *srcVPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(V_Buffer));
/*
    if (srcYPtr == nullptr)
    {
        LOGE("srcYPtr null ERROR!");
        return;
    }
    else if (srcUPtr == nullptr)
    {
        LOGE("srcUPtr null ERROR!");
        return;
    }
    else if (srcVPtr == nullptr)
    {
        LOGE("srcVPtr null ERROR!");
        return;
    }
*/
    ANativeWindow * window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_acquire(window);
    ANativeWindow_Buffer buffer;
    //set output size and format
    //only 3 formats are available:
    //WINDOW_FORMAT_RGBA_8888(DEFAULT), WINDOW_FORMAT_RGBX_8888, WINDOW_FORMAT_RGB_565
    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);
    if (int32_t err = ANativeWindow_lock(window, &buffer, NULL)) {
//        LOGE("ANativeWindow_lock failed with error code: %d\n", err);
        ANativeWindow_release(window);
    }

    size_t bufferSize = buffer.width * buffer.height * (size_t)4;

    //YUV_420_888 to RGBA_8888 conversion and flip
    uint8_t * outPtr = reinterpret_cast<uint8_t *>(buffer.bits);
    for (size_t y = 0; y < srcHeight; y++)
    {
        uint8_t * Y_rowPtr = srcYPtr + y * Y_rowStride;
        uint8_t * U_rowPtr = srcUPtr + (y >> 1) * UV_rowStride;
        uint8_t * V_rowPtr = srcVPtr + (y >> 1) * UV_rowStride;
        for (size_t x = 0; x < srcWidth; x++)
        {
            uint8_t Y = Y_rowPtr[x];
            uint8_t U = U_rowPtr[(x >> 1)];
            uint8_t V = V_rowPtr[(x >> 1)];
            //from Wikipedia article YUV:
            //Integer operation of ITU-R standard for YCbCr(8 bits per channel) to RGB888
            //Y-Y, U-Cb, V-Cr
            //U -= 128
            //V -= 128
            //R = Y + V + (V >> 2) + (V >> 3) + (V >> 5)
            //  = Y + V * 1.40625;
            //G = Y - ((U >> 2) + (U >> 4) + (U >> 5)) - ((V >> 1) + (V >> 3) + (V >> 4) + (V >> 5))
            //  = Y - (U - 128) * 0.34375 - (V - 128) * 0.71875;
            //B = Y + U + (U >> 1) + (U >> 2) + (U >> 6)
            //  = Y + (U - 128) * 1.765625;
            double R = (Y + (V - 128) * 1.40625);
            double G = (Y - (U - 128) * 0.34375 - (V - 128) * 0.71875);
            double B = (Y + (U - 128) * 1.765625);
            *(outPtr + (--bufferSize)) = 255; // gamma for RGBA_8888
            *(outPtr + (--bufferSize)) = (uint8_t) (B > 255 ? 255 : (B < 0 ? 0 : B));
            *(outPtr + (--bufferSize)) = (uint8_t) (G > 255 ? 255 : (G < 0 ? 0 : G));
            *(outPtr + (--bufferSize)) = (uint8_t) (R > 255 ? 255 : (R < 0 ? 0 : R));
        }
    }

    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);
}

//using another conversion method offered by @alijandro at a question in stackoverflow
//https://stackoverflow.com/questions/46087343/jni-yuv-420-888-to-rgba-8888-conversion
JNIEXPORT void JNICALL Java_tau_camera2demo_JNIUtils_RGBADisplay2(
        JNIEnv *env,
        jobject obj,
        jint srcWidth,
        jint srcHeight,
        jint Y_rowStride,
        jobject Y_Buffer,
        jobject U_Buffer,
        jobject V_Buffer,
        jobject surface) {

    uint8_t *srcYPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(Y_Buffer));
    uint8_t *srcUPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(U_Buffer));
    uint8_t *srcVPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(V_Buffer));
/*
    if (srcYPtr == nullptr)
    {
        LOGE("srcYPtr null ERROR!");
        return;
    }
    else if (srcUPtr == nullptr)
    {
        LOGE("srcUPtr null ERROR!");
        return;
    }
    else if (srcVPtr == nullptr)
    {
        LOGE("srcVPtr null ERROR!");
        return;
    }
*/
    ANativeWindow * window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_acquire(window);
    ANativeWindow_Buffer buffer;
    //set output size and format
    //only 3 formats are available:
    //WINDOW_FORMAT_RGBA_8888(DEFAULT), WINDOW_FORMAT_RGBX_8888, WINDOW_FORMAT_RGB_565
    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);
    if (int32_t err = ANativeWindow_lock(window, &buffer, NULL)) {
//        LOGE("ANativeWindow_lock failed with error code: %d\n", err);
        ANativeWindow_release(window);
    }

    size_t bufferSize = buffer.width * buffer.height * (size_t)4;

    //YUV_420_888 to RGBA_8888 conversion
    uint8_t * outPtr = reinterpret_cast<uint8_t *>(buffer.bits);
    for (size_t y = 0; y < srcHeight; y++)
    {
        uint8_t * Y_rowPtr = srcYPtr + y * Y_rowStride;
        uint8_t * U_rowPtr = srcUPtr + (y >> 1) * Y_rowStride / 2;
        uint8_t * V_rowPtr = srcVPtr + (y >> 1) * Y_rowStride / 2;
        for (size_t x = 0; x < srcWidth; x++)
        {
            //from Wikipedia article YUV:
            //Integer operation of ITU-R standard for YCbCr(8 bits per channel) to RGB888
            //Y-Y, U-Cb, V-Cr
            //R = Y + V + (V >> 2) + (V >> 3) + (V >> 5);
            //G = Y - ((U >> 2) + (U >> 4) + (U >> 5)) - ((V >> 1) + (V >> 3) + (V >> 4) + (V >> 5));
            //B = Y + U + (U >> 1) + (U >> 2) + (U >> 6);
            uint8_t Y = Y_rowPtr[x];
            uint8_t U = U_rowPtr[(x >> 1)];
            uint8_t V = V_rowPtr[(x >> 1)];
            double R = ((Y-16) * 1.164 + (V-128) * 1.596);
            double G = ((Y-16) * 1.164 - (U-128) * 0.392 - (V-128) * 0.813);
            double B = ((Y-16) * 1.164 + (U-128) * 2.017);
            *(outPtr + (--bufferSize)) = 255; // gamma for RGBA_8888
            *(outPtr + (--bufferSize)) = (uint8_t) (B > 255 ? 255 : (B < 0 ? 0 : B));
            *(outPtr + (--bufferSize)) = (uint8_t) (G > 255 ? 255 : (G < 0 ? 0 : G));
            *(outPtr + (--bufferSize)) = (uint8_t) (R > 255 ? 255 : (R < 0 ? 0 : R));
        }
    }

    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);
}
}

