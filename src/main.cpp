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

int min_hue = 0, max_hue = 255, min_sat = 0, max_sat = 255;

void minhue(int, void*)
{
	printf("min hue = %d\n", min_hue);
}

void maxhue(int, void*)
{
}

void minsat(int, void*)
{
}

void maxsat(int, void*)
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

			if(pixel[0] >= min_hue && pixel[0] <= max_hue && pixel[1] >= min_sat && pixel[1] <= max_sat)
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

void calcProbabMask(Mat* frame, Mat mean, Mat* covar, float covarDet, Mat* out)
{
	float* data = (float*)frame->data;
	//float* out_data = (float*)out->data;
	int channels = frame->channels();
	static Mat val(Size(1,1), CV_32FC1);
	unsigned int i = 0, j = 0;
	static float P;
	static unsigned int f_cols = frame->cols, f_rows = frame->rows;
	float factor = 1 / sqrt(DPISQRD * covarDet);
	double hue = 0, saturation = 0;

	// strasznie du¿o czasu spêdza na iteracji po pikselach
	for(; i < f_rows; i++)
	{
		for(j = 0; j < f_cols; j++)
		{
			Scalar pixel(	data[channels * (f_cols*i + j)],
							data[channels * (f_cols*i + j) + 1]		);
			
			// to-do: tworzyæ pixel wczeœniej, a tu tylko uzupe³niaæ

			val = (pixel - mean) * *covar * (pixel - mean).t();
			P = factor * exp(-0.5f * val.data[0]);
			//P = 1;
			out->at<float>(i, j) = P;
			//out->data[channels * (frame->cols*i + j)] = 255; // czy to na pewno bêdzie floatem? // temporary: *255
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

	createTrackbar("Min Hue", "Suwaki", &min_hue, 255, minhue);
	createTrackbar("Max Hue", "Suwaki", &max_hue, 255, maxhue);
	createTrackbar("Min Sat", "Suwaki", &min_sat, 255, minsat);
	createTrackbar("Max Sat", "Suwaki", &max_sat, 255, maxsat);

	BackgroundSubtractorMOG2 sub;
	sub.set("nmixtures", 3);
	sub.set("detectShadows", false);
	int frames = 500;

	unsigned char size = 2;
	Mat elem = getStructuringElement(	MORPH_ELLIPSE,
										Size(2*size+1, 2*size+1),
										Point(size, size)
										);

    do {
        capture >> frame;
        flip(frame, frame, 1);
		//cvtColor(frame, effects, CV_BGR2HSV);
		
		if(frames > 0)
		{
			sub(frame, foreground);
			frames--;
		}
		else
		{
			sub(frame, foreground, 0);
		}
		sub.getBackgroundImage(effects);
		erode(foreground, foreground, Mat());
		dilate(foreground, foreground, Mat());
		//morphologyEx(foreground, foreground, MORPH_OPEN, elem);


		//effects.convertTo(effects, CV_8UC3);

		//getMask(&effects, &mask);
		//probab.convertTo(probab, CV_8UC3);
		imshow("Effects", foreground); // effects
        imshow("CAM", frame); 

        key = waitKey(1);
    } while (key != KEY_ESCAPE);

    return 0;
}