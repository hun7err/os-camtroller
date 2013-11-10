#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;

#define KEY_ESCAPE 27

int main()
{
    int key = -1;

    VideoCapture capture(0);
    if(!capture.isOpened())
    {
        cout << "Could not open the camera device, quitting." << endl;
        return -1;
    }

    namedWindow("CAM", CV_WINDOW_AUTOSIZE);
    namedWindow("Effects", CV_WINDOW_AUTOSIZE);

    Mat frame, effects;

    int canny_threshold = 30;

    Mat channels[3], hsv;

    do {
        capture >> frame;
        flip(frame, frame, 1);
        //medianBlur(frame, frame, 5);
        cvtColor(frame, effects, CV_BGR2YCrCb); // CV_BGR2HSV
        for(int i=0;i<3;i++) channels[i] = Mat::zeros(channels[i].size(), CV_8U);
        split(effects, channels);
        channels[0] = Mat::zeros(channels[0].size(), CV_8U);
        effects = Mat::zeros(effects.size(), CV_8UC3);
        merge(channels, 3, effects);
        cvtColor(effects, effects, CV_HSV2BGR);

        Canny(effects, effects, canny_threshold, canny_threshold*2, 3);

        imshow("CAM", frame);
        imshow("Effects", effects);

        key = waitKey(1);
    } while (key != KEY_ESCAPE);

    return 0;
}
