#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef WIN32
	#include <Windows.h>
#endif
using namespace cv;
using namespace std;

const double EulerConstant = std::exp(1.0);
const double DPISQRD = std::pow((2*M_PI), 2.0);

#define KEY_ESCAPE 27
#define KEY_SPACE  32

//int min_hue = 0, max_hue = 255, min_sat = 0, max_sat = 255;
int min_h = 6, max_h = 38, min_s = 38, max_s = 250, min_v = 51, max_v = 242;


void minh(int, void*)
{
}

void maxh(int, void*)
{
}

void mins(int, void*)
{
}

void maxs(int, void*)
{
}

void minv(int, void*)
{
}

void maxv(int, void*)
{
}

void getMask(Mat* frame, Mat* mask)
{
	static unsigned int i = 0, j = 0;
	static const unsigned int rows = frame->rows, cols = frame->cols;
	static Vec3b pixel;

	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < cols; j++)
		{
			//printf("i = %d, j = %d\n", i, j);
			pixel = frame->at<Vec3b>(i, j);

			//if(pixel[0] >= min_hue && pixel[0] <= max_hue && pixel[1] >= min_sat && pixel[1] <= max_sat)
			if(pixel[0] > min_h && pixel[0] < max_h && pixel[1] > min_s && pixel[1] < max_s && pixel[2] > min_v && pixel[2] < max_v)
			{
				mask->at<unsigned char>(i,j) = 255;
			}
			else
			{
				mask->at<unsigned char>(i,j) = 0;
			}
		}
	}
}

int main()
{
    int key = -1;

    VideoCapture capture(0);

	#ifdef WIN32
		Sleep(1000);
	#endif
	//capture.set(CV_CAP_PROP_EXPOSURE, 3.0);

	if(!capture.isOpened())
    {
        cout << "Could not open the camera device, quitting." << endl;
        return -1;
    }
	capture.set(CV_CAP_PROP_AUTO_EXPOSURE, 0);
	capture.set(CV_CAP_PROP_SETTINGS, 1);
	capture.set(CV_CAP_PROP_BACKLIGHT, 0);

	printf("OCV version: %d.%d\n", CV_MAJOR_VERSION, CV_MINOR_VERSION);

    cv::namedWindow("CAM", CV_WINDOW_AUTOSIZE);

    Mat frame, effects, foreground;

    Mat channels[3], hsv;
	int rect_width = 150, rect_height = 150, line_thickness = 2;
	double	rect_x = (capture.get(CV_CAP_PROP_FRAME_WIDTH) - rect_width)/2 - line_thickness,
			rect_y = (capture.get(CV_CAP_PROP_FRAME_HEIGHT) - rect_height)/2 - line_thickness;

	capture >> frame;	// odczytujemy klatke
	Mat mask = Mat::zeros(frame.size(), CV_8UC1);	// tworzymy czarny obraz wielkoœci odczytanej klatki
	
	vector<Mat> chnls;

	SetCursorPos(500, 500);
	// http://msdn.microsoft.com/en-us/library/ms646273%28v=vs.85%29.aspx
	//MOUSEINPUT mi = {1, 1, XBUTTON1, MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, 0, GetMessageExtraInfo()};
	//INPUT input = {INPUT_MOUSE, mi};
	//SendInput(1, &input, sizeof(input));
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//SetCursorPos(1336, 27);
	//mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP, 0, 0, XBUTTON1, GetMessageExtraInfo());
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    cv::namedWindow("Effects", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Suwaki", CV_WINDOW_AUTOSIZE);

	/*
	createTrackbar("Min Hue", "Suwaki", &min_hue, 255, minhue);
	createTrackbar("Max Hue", "Suwaki", &max_hue, 255, maxhue);
	createTrackbar("Min Sat", "Suwaki", &min_sat, 255, minsat);
	createTrackbar("Max Sat", "Suwaki", &max_sat, 255, maxsat);
	*/
	createTrackbar("Min Hue", "Suwaki", &min_h, 255, minh);
	createTrackbar("Max Hue", "Suwaki", &max_h, 255, maxh);
	createTrackbar("Min Saturation", "Suwaki", &min_s, 255, mins);
	createTrackbar("Max Saturation", "Suwaki", &max_s, 255, maxs);
	createTrackbar("Min Value", "Suwaki", &min_v, 255, minv);
	createTrackbar("Max Value", "Suwaki", &max_v, 255, maxv);

	BackgroundSubtractorMOG2 sub;
	sub.set("nmixtures", 3);
	sub.set("detectShadows", false);
	int frames = 0; // 500

	unsigned char size = 1;
	Mat elem = getStructuringElement(	MORPH_ELLIPSE,
										Size(2*size+1, 2*size+1),
										Point(size, size)
										);

	vector<vector<Point> > kontury;
	foreground = Mat::zeros(frame.size(), CV_8UC1);

    do {
        capture >> frame;
        flip(frame, frame, 1);
		//cvtColor(frame, effects, CV_BGR2HSV);
		
			//sub(frame, foreground);
			/*
		if(frames > 0)
		{
			sub(frame, foreground);
			//frames--;
		}
		else
		{
			sub(frame, foreground, 0);
		}*/
		//sub.getBackgroundImage(effects);
		cvtColor(frame, effects, CV_BGR2HSV);
		getMask(&effects, &mask);
		GaussianBlur(mask, mask, Size(5,5), 5, 5);
		erode(mask, mask, elem);
		dilate(mask, mask, elem);
		mask.copyTo(foreground);
		

		findContours(foreground, kontury, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // APPROX_SIMPLE
		for(vector<vector<Point> >::iterator it = kontury.begin(); it != kontury.end(); ++it)
		{
			if(contourArea(*it) >= 5000)
			{
				vector<vector<Point> > tcontours;
				tcontours.push_back(*it);
				drawContours(frame,tcontours,-1,Scalar(0,0,255),2);

				vector<vector<Point> > hulls(1);
				vector<vector<int> > hullsI(1);
				convexHull(Mat(tcontours[0]),hulls[0],false);
				convexHull(Mat(tcontours[0]),hullsI[0],false);
				drawContours(frame,hulls,-1,Scalar(0,255,0),2);
			}
		}
		//morphologyEx(foreground, foreground, MORPH_OPEN, elem);


		//effects.convertTo(effects, CV_8UC3);

		//getMask(&effects, &mask);
		//probab.convertTo(probab, CV_8UC3);
		imshow("Effects", mask); // foreground
        imshow("CAM", frame); 

        key = waitKey(1);
		kontury.clear();
    } while (key != KEY_ESCAPE);

    return 0;
}