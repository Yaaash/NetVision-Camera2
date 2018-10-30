#include <jni.h>
#include <opencv2/opencv.hpp>
#include <GLES2/gl2.h>

#include "stdio.h"
#include <string>
#include <vector>

using namespace cv;
using namespace std;

extern "C" JNIEXPORT void
JNICALL
Java_com_netvirta_netvisioncamera2_MyGLSurfaceView_processFrame(JNIEnv *env, jobject, jint texOut, jint width, jint height,
                                                                jboolean frontFacing) {
    static UMat frame;

    frame.create(height, width, CV_8UC4);

    // read
    // expecting FBO to be bound, read pixels to mat
    glReadPixels(0, 0, frame.cols, frame.rows, GL_RGBA, GL_UNSIGNED_BYTE, frame.getMat(ACCESS_WRITE).data);

    // Check if we should flip image due to frontFacingVar
    if (frontFacing) {
        flip(frame, frame, 1);
    }

    Mat destination;

    // Detect the edges of the image by using a Canny detector
    Canny(frame, destination, 50, 200, 3);

    vector<Vec4i> lines;
    HoughLinesP(destination, lines, 1, CV_PI / 180, 50, 50, 10);
    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i l = lines[i];
        //  display the result by drawing the lines.
        cv::line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
    }

    // write back
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texOut);

    // show UI via OpenGLS
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, GL_RGBA, GL_UNSIGNED_BYTE, frame.getMat(ACCESS_READ).data);
}