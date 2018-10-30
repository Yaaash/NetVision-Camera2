//
// Created by Yashika on 28-10-2018.
//
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/features2d/features2d.hpp>

#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

using namespace std;
using namespace cv;

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

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Camera2-Native", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Camera2-Native", __VA_ARGS__)


#define DEBUG 1
#define IMAGE_FORMAT_YV12 842094169
#define PI 3.14159

/**
 * Round x up to a multiple of mask.
 *  E.g., ALIGN(x, 16) means round x up to the nearest multiple of 16.
 */
#define ALIGN(x, mask) (((x) + (mask)-1) & ~((mask)-1))

typedef struct b_line_ {
    float m_alpha;
    int m_dis;
    int m_lr; // left or right 0-left,  1-right
} b_line;

#ifndef NETVISION_CAMERA2_NATIVE_LIB_H
#define NETVISION_CAMERA2_NATIVE_LIB_H
#ifdef __cplusplus
extern "C" {
#endif
/*
   * Class:     com_netvirta_netvisioncamera2_JNIUtils
   * Method:    detectLane
   * Signature: (JJ)I
   */
JNIEXPORT jstring JNICALL com_netvirta_netvisioncamera2_JNIUtils_detectLane(
        JNIEnv *, jobject, jint, jint,
        jobject, jobject);

int calc_distance(int width, int height, Point p0, Point p1, float &alpha, int &lr);

void LaneDetect(Mat &img_rgba, char *outStr);

#ifdef __cplusplus
}
#endif
#endif

