#include <jni.h>
#include "native-lib.h"

JNIEXPORT jstring JNICALL com_netvirta_netvisioncamera2_JNIUtils_detectLane(
        JNIEnv *env, jobject obj, jint srcWidth, jint srcHeight,
        jobject srcBuffer, jobject dstSurface) {

    char outStr[200];

    uint8_t *srcLumaPtr = reinterpret_cast<uint8_t *>(env->GetDirectBufferAddress(srcBuffer));

    if (srcLumaPtr == nullptr) {
        LOGE("blit NULL pointer ERROR");
        return NULL;
    }

    int dstWidth;
    int dstHeight;

    cv::Mat mYuv(srcHeight + srcHeight / 2, srcWidth, CV_8UC1, srcLumaPtr);

    ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
    ANativeWindow_acquire(win);

    ANativeWindow_Buffer buf;

    dstWidth = srcHeight;
    dstHeight = srcWidth;

    ANativeWindow_setBuffersGeometry(win, dstWidth, dstHeight, 0 /*format unchanged*/);

    if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
        LOGE("ANativeWindow_lock failed with error code %d\n", err);
        ANativeWindow_release(win);
        return NULL;
    }

    uint8_t *dstLumaPtr = reinterpret_cast<uint8_t *>(buf.bits);
    Mat dstRgba(dstHeight, buf.stride, CV_8UC4,
                dstLumaPtr);        // TextureView buffer, use stride as width
    Mat srcRgba(srcHeight, srcWidth, CV_8UC4);
    Mat flipRgba(dstHeight, dstWidth, CV_8UC4);

    // convert YUV -> RGBA
    cv::cvtColor(mYuv, srcRgba, CV_YUV2RGBA_NV21);

    // Rotate 90 degree
    cv::transpose(srcRgba, flipRgba);
    cv::flip(flipRgba, flipRgba, 1);


    LaneDetect(flipRgba, outStr);

    // copy to TextureView surface
    uchar *dbuf;
    uchar *sbuf;
    dbuf = dstRgba.data;
    sbuf = flipRgba.data;
    int i;
    for (i = 0; i < flipRgba.rows; i++) {
        dbuf = dstRgba.data + i * buf.stride * 4;
        memcpy(dbuf, sbuf, flipRgba.cols * 4);
        sbuf += flipRgba.cols * 4;
    }

    // Draw some rectangles
    Point p1(100, 100);
    Point p2(300, 300);
    cv::rectangle(dstRgba, p1, p2, Scalar(255, 255, 255));
    cv::rectangle(dstRgba, Point(10, 10), Point(dstWidth - 1, dstHeight - 1),
                  Scalar(255, 255, 255));
    cv::rectangle(dstRgba, Point(100, 100), Point(dstWidth / 2, dstWidth / 2),
                  Scalar(255, 255, 255));

    LOGE("bob dstWidth=%d height=%d", dstWidth, dstHeight);
    ANativeWindow_unlockAndPost(win);
    ANativeWindow_release(win);

    return env->NewStringUTF(outStr);
}


void LaneDetect(Mat &img_rgba, char *outStr) {
    Mat img_hsv;
    Mat img_3;
    Mat img_dis;

    cv::Mat img_gray(img_rgba.size(), CV_8UC1);
    cv::cvtColor(img_rgba, img_hsv, CV_RGB2HSV);

    int h_max;
    int h_min;
    int s_max;
    int s_min;
    int v_max;
    int v_min;

    uchar *buf;
    buf = img_hsv.data;

    h_max = 100;
    h_min = 50;
    s_max = 240;
    s_min = 20;
    v_max = 255;
    v_min = 110;

    int i;
    int j;
    for (i = 0; i < img_hsv.rows; i++) {
        for (j = 0; j < img_hsv.cols; j++) {
            uchar *b;
            b = buf + i * img_hsv.cols * 3 + j * 3;
            if (((b[0] <= h_max) && (b[0] >= h_min) && ((b[1] <= s_max) && (b[1] >= s_min)) &&
                 ((b[2] <= v_max) && (b[2] >= v_min)))) {
                img_gray.at<uchar>(i, j) = 255;
            } else {
                img_gray.at<uchar>(i, j) = 0;
            }
        }
    }

    img_dis = img_gray;
    cvtColor(img_dis, img_3, COLOR_GRAY2RGB);
    cv::cvtColor(img_3, img_rgba, CV_RGB2RGBA);

    cv::blur(img_gray, img_gray, Size(15, 15));
    threshold(img_gray, img_gray, 100, 255, CV_THRESH_BINARY);

#if 1
    Mat img_contours;
    Canny(img_gray, img_contours, 50, 250);

    vector<Vec4i> lines;
    HoughLinesP(img_contours, lines, 1, CV_PI / 180, 40, 100, 30);

//    LOGE("bob lines count:%d", lines.size());
    float alpha;
    int width;
    int height;
    int distance;
    int lr = 0;

    b_line m_lines[10];
    int line_count;
    width = img_rgba.cols;
    height = img_rgba.rows;
    line_count = 0;
    for (i = 0; i < 10; i++) {
        m_lines[i].m_alpha = 0.0;
        m_lines[i].m_dis = -1;
    }
    for (i = 0; i < lines.size(); i++) {
        line(img_rgba, Point(lines[i][0], lines[i][1]),
             Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 3, 8);

        distance = calc_distance(width, height, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), alpha,
                                 lr);
        if (m_lines[0].m_dis == -1) {
            m_lines[0].m_dis = distance;
            m_lines[0].m_alpha = alpha;
            m_lines[0].m_lr = lr;
            line_count = 1;
        } else {
            int j;
            int findit = 0;
            for (j = 0; j < line_count; j++) {
                if ((m_lines[j].m_alpha < alpha + 5) && (m_lines[j].m_alpha > alpha - 5)) {
                    if (m_lines[j].m_dis > distance) {
                        m_lines[j].m_dis = distance;
                        m_lines[j].m_alpha = alpha;
                        m_lines[j].m_lr = lr;
                    }
                    findit = 1;
                    break;
                } else {

                }
            }
            if (!findit) {
                m_lines[line_count].m_alpha = alpha;
                m_lines[line_count].m_dis = distance;
                m_lines[line_count].m_lr = lr;
                line_count++;
            }
        }

        LOGE("bob lines dis (%d,%d) - (%d,%d) alpha:%f distance:%d\n", lines[i][0], lines[i][1], lines[i][2],
             lines[i][3], alpha, distance);
    }
    LOGE("bob lines ------\n");
    char *pbuf = outStr;
    sprintf(pbuf, "count=%d;", line_count);
    pbuf += strlen(pbuf);
    for (i = 0; i < line_count; i++) {
        LOGE("bob lines result:%d alpha:%f dis:%d lr:%d\n", i, m_lines[i].m_alpha, m_lines[i].m_dis, m_lines[i].m_lr);
        sprintf(pbuf, "(%d,%d,%d);", (int) m_lines[i].m_alpha, m_lines[i].m_dis, m_lines[i].m_lr);
        pbuf += strlen(pbuf);
    }
    LOGE("bob line detect done");
#endif
}

int calc_distance(int width, int height, Point p0, Point p1, float &alpha, int &lr) {

    int x0;
    int y0;
    int x1;
    int y1;
    int x_org;
    int y_org;
    float a;
    float b;

    lr = 0;
    x_org = width / 2;
    y_org = height;

    x0 = p0.x - x_org;
    y0 = y_org - p0.y;

    x1 = p1.x - x_org;
    y1 = y_org - p1.y;

    int d;

    if ((x1 != x0) && (x1 != 0)) {
        a = (float) (y1 - y0);
        a = a / ((float) (x1 - x0));
        b = y0 - a * (float) x0;
        alpha = atan(a);
        d = (int) ((b / a) * sin(alpha));

        alpha *= 180 / PI;
        if (((b > 0) && (a < 0)) || ((b < 0) && (a > 0))) {
            lr = 1;
        } else {
            lr = 0;
        }

    } else {
        a = 0;
        b = 0;
        alpha = 90;
        d = x1;
        if (d > 0) {
            lr = 1;
        } else {
            lr = 0;
        }
    }

    d = abs(d);
    return d;
}

