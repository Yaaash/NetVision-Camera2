#include <jni.h>
#include "com_netvirta_netvisioncamera2_JNIUtils.h"
#include "opencv2/imgcodecs.hpp"

JNIEXPORT jstring JNICALL Java_com_netvirta_netvisioncamera2_JNIUtils_detectLine
        (JNIEnv *env, jclass, jint srcWidth, jint srcHeight, jobject srcBuffer, jobject surface, jstring path,
         jlong destinationAddress) {

    Mat &finalDestination = *(Mat *) destinationAddress;

    const char *filePathNativeString = env->GetStringUTFChars(path, NULL);

    // My Frame is YUV format data, and it's buffer is:
    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));

    if (srcLumaPtr == nullptr) {
        // pointer is null for buffer
        return (env)->NewStringUTF("Src is Null");
    }
    // create YUV mat file from buffer
    cv::Mat mYuv(srcHeight + srcHeight / 2, srcWidth, CV_8UC1, srcLumaPtr);

    int dstWidth;
    int dstHeight;
    // match the width of destination with source, same for height
    dstWidth = srcHeight;
    dstHeight = srcWidth;

    ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
//    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;

    ANativeWindow_setBuffersGeometry(win, srcWidth, srcHeight, 0 /*format unchanged*/);

    // create buffer for destination
    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4, dstLumaPtr);        // TextureView buffer, use stride as width

    Mat srcRgba(srcHeight, srcWidth, CV_8UC4);

    // convert YUV to RGBA
    cv::cvtColor(mYuv, srcRgba, CV_YUV2RGBA_NV21);
    // Rotate 90 degree
    cv::transpose(srcRgba, finalDestination);
    cv::flip(finalDestination, finalDestination, 1);

    // currently flipRgba has output
    // .....line detection
    // Detect the edges of the image by using a Canny detector
    Mat cannyDestination;
    Canny(finalDestination, cannyDestination, 50, 200, 3);

    vector<Vec4i> lines;
    HoughLinesP(cannyDestination, lines, 1, CV_PI / 180, 50, 50, 10);
    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i l = lines[i];
        //  display the result by drawing the lines.
        cv::line(finalDestination, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 3, CV_AA);
    }

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
        // Couldn't lock ANativeWindow
        ANativeWindow_release(win);
        __android_log_print(ANDROID_LOG_ERROR, "yashika", "ANativeWindow_lock failed with error code %d", err);
        return (env)->NewStringUTF("ANativeWindow_lock failed");
    }

    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);
    return env->NewStringUTF(reinterpret_cast<const char *>(lines.size()));
}
