#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef WIN32
	#include <Windows.h>
#endif
using namespace cv;
using namespace std;

#define KEY_ESCAPE 27
#define KEY_SPACE  32

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

int main()
{
    int key = -1;

    VideoCapture capture(0);

	#ifndef WIN32
		Sleep(1000);	// opcjonalne opóŸnienie, brak mo¿e powodowaæ dziwne b³êdy (w tym czasie nastêpuje komunikacja z urz¹dzeniem)
	#endif

	if(!capture.isOpened())
    {
        cout << "Could not open the camera device, quitting." << endl;
        return -1;
    }
	capture.set(CV_CAP_PROP_SETTINGS, 1);	// poka¿ okno z ustawieniami kamery

    cv::namedWindow("CAM", CV_WINDOW_AUTOSIZE);
	Mat frame, copy;
	
	CascadeClassifier fist("fist.xml"), hand("haarcascade.xml");
	std::vector<cv::Rect> hands_open, fists;

	double cur_x, cur_y;

    do {
        capture >> frame;
        flip(frame, frame, 1);
		cvtColor(frame, copy, COLOR_BGR2GRAY);
		equalizeHist(copy, copy);
		hand.detectMultiScale(copy, hands_open);
		fist.detectMultiScale(copy, fists);

		if(!hands_open.empty())
			cur_x = cur_y = 0;

		int max_palm_index = 0;

		for(int i = 0; i < hands_open.size(); i++)
		{
			if(hands_open[i].area() > hands_open[max_palm_index].area())
				max_palm_index = i;

			cur_x += hands_open[i].x + hands_open[i].width/2;
			cur_y += hands_open[i].y + hands_open[i].height/2;
		}

		int max_index = 0;
		if(!fists.empty())
			cur_x = cur_y = 0;

		for(int i = 0; i < fists.size(); i++)
		{
			if(fists[i].area() > fists[max_index].area())
				max_index = i;
			cur_x += fists[i].x + fists[i].width/2;
			cur_y += fists[i].y + fists[i].height/2;
		}

		if(hands_open.empty())
		{
			cur_x = cur_y = -1;
		}
		else
		{
			if(hands_open[max_palm_index].area() > 10000)
				rectangle(frame, hands_open[max_palm_index], cv::Scalar(255,0,0));
			cur_x /= hands_open.size();
			cur_y /= hands_open.size();

			cur_x = cur_x * SCREEN_WIDTH / frame.size().width;
			cur_y = cur_y * SCREEN_HEIGHT / frame.size().height;

			SetCursorPos(cur_x, cur_y);
		}

		if(!fists.empty())
		{
			cur_x /= fists.size();
			cur_y /= fists.size();

			cur_x = cur_x * SCREEN_WIDTH / frame.size().width;
			cur_y = cur_y * SCREEN_HEIGHT / frame.size().height;

			SetCursorPos(cur_x, cur_y);

			if(fists[max_index].area() > 5000)
				rectangle(frame, fists[max_index], cv::Scalar(0, 0, 255));

			mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP, 0, 0, XBUTTON1, GetMessageExtraInfo());
		}
        imshow("CAM", frame); 

        key = waitKey(1);
    } while (key != KEY_ESCAPE);

    return 0;
}