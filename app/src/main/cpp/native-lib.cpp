#include <jni.h>
#include <string>
#include "opencv2/opencv.hpp"

#include <string>
#include <vector>

#include <android/log.h>

using namespace cv;

/**
 * Method to detect frame
 */
void detect(Mat &frame);

extern "C" JNIEXPORT jstring JNICALL
Java_com_netvirta_netvisioncamera2_CameraActivity_faceDetection(
        JNIEnv *, jclass, jlong rgbaAddress) {

    Mat &frame = *(Mat *) rgbaAddress;
    detect(frame);
}


void detect(Mat &frame) {

    String face_cascade_name = "/sdcard/Download/cascade/haarcascade_frontalface_alt.xml";
    String eyes_cascade_name = "/sdcard/Download/cascade/haarcascade_eye_tree_eyeglasses.xml";
    //syslog(LOG_CRIT, "hello syslog");
    CascadeClassifier face_cascade;
    CascadeClassifier eyes_cascade;
    if (!face_cascade.load(face_cascade_name)) {/* printf("--(!)Error loading\n")*/;
        return;
    };
    if (!eyes_cascade.load(eyes_cascade_name)) {/* printf("--(!)Error loading\n")*/;
        return;
    };


    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //-- Detect faces
//    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for (size_t i = 0; i < faces.size(); i++) {
        Point center(faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5);
        ellipse(frame, center, Size(faces[i].width * 0.5, faces[i].height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8,
                0);

        Mat faceROI = frame_gray(faces[i]);
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
//        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

        circle(frame, Point(0,0), 10, Scalar(255, 0, 0), 4, 8, 0);
        for (size_t j = 0; j < eyes.size(); j++) {
            Point eye_center(faces[i].x + eyes[j].x + eyes[j].width * 0.5, faces[i].y + eyes[j].y + eyes[j].height * 0.5);
            int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
            circle(frame, eye_center, radius, Scalar(255, 0, 0), 4, 8, 0);
        }
    }

}