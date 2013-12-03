

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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

    Mat frame, effects;

    Mat channels[3], hsv;
	int rect_width = 150, rect_height = 150, line_thickness = 2;
	double	rect_x = (capture.get(CV_CAP_PROP_FRAME_WIDTH) - rect_width)/2 - line_thickness,
			rect_y = (capture.get(CV_CAP_PROP_FRAME_HEIGHT) - rect_height)/2 - line_thickness;

	capture >> frame;	// odczytujemy klatke
	Mat mask = Mat::zeros(frame.size(), CV_8U);	// tworzymy czarny obraz wielkoœci odczytanej klatki
	rectangle(mask, Point(rect_x,rect_y), Point(rect_x+rect_width+2*line_thickness, rect_y+rect_height+2*line_thickness), Scalar(255,255,255), -1); // ignorujemy obszar, który nie bêdzie rozmyty
	bitwise_not(mask, mask);	// chcemy mieæ bia³e t³o ¿eby wyci¹æ tylko kwadrat

	Mat blurred = Mat::zeros(frame.size(), CV_8UC3);	// tu bedzie rozmywana czêœæ obrazu
	const int sample_count = 1; // 8
	
	int cur_sample_id = 0;
	bool sample_gathering = false;

	Mat placeholder = Mat::zeros(Size(rect_width, rect_height), CV_32FC3);
	Mat sample = Mat::zeros(placeholder.size(), CV_32FC2), sample_channels[3];
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

	do {
        capture >> frame;
        flip(frame, frame, 1);
		frame.copyTo(blurred, mask);
		GaussianBlur(blurred, blurred, Size(25, 25), 10, 10);
		blurred.copyTo(frame, mask);
		
		rectangle(frame, Point(rect_x,rect_y), Point(rect_x+rect_width+2*line_thickness, rect_y+rect_height+2*line_thickness), Scalar(0,0,255), line_thickness);
        imshow("CAM", frame);

		/*POINT p;
		if (GetCursorPos(&p))
		{
			printf("(%d,%d)\n", p.x, p.y);
		}*/

        key = waitKey(1);
		if(key == KEY_SPACE)
		{
			sample_gathering = true;
		}
		if(sample_gathering)
		{
			//samples[cur_sample_id] = Mat::zeros(Size(rect_width, rect_height), CV_8UC3);
			
			//Mat(frame, Rect(rect_x+line_thickness, rect_y+line_thickness, rect_width, rect_height)).copyTo(samples[cur_sample_id]);
			//cvtColor(samples[cur_sample_id], samples[cur_sample_id], CV_BGR2HSV);
			
			
			Mat(frame, Rect(rect_x+line_thickness, rect_y+line_thickness, rect_width, rect_height)).copyTo(placeholder);
			cvtColor(placeholder, placeholder, CV_BGR2HSV);
			placeholder.convertTo(sample, CV_32FC2);
			//placeholder = frame;

			//split(placeholder, sample_channels);
			//merge(sample_channels, 2, sample);

			//samples[cur_sample_id][0] = chnls[0];
			//merge(&chnls[0], 2, samples[cur_sample_id]);
			break;
		}
    } while (1);
	
	/*Mat sample = Mat::zeros(Size(rect_width, rect_height), CV_8UC3);
	Mat(frame, Rect(rect_x+line_thickness, rect_y+line_thickness, rect_width, rect_height)).copyTo(sample);
	cvtColor(sample, sample, CV_BGR2HSV);*/
	
    cv::namedWindow("Effects", CV_WINDOW_AUTOSIZE);

	Mat probab;

	Scalar mean_value = mean(sample);
	double mean_h = mean_value[0], mean_s = mean_value[1];
	Mat P = Mat::zeros(Size(2, sample.size().width*sample.size().height), CV_32F);
	unsigned int i = 0, j = 0;
	printf("for()\n");
	unsigned int s_channels = sample.channels(), s_cols = sample.cols, s_rows = sample.rows;

	Vec2f pixel;

	for(; i < s_rows; i++) {
		for(j = 0; j < s_cols; j++)
		{ 
			/*P.at<float>(i, j) = sample.data[s_channels * (s_cols*i + j)] - mean_h - 0.5;
			P.at<float>(i, j+1) = sample.data[s_channels * (s_cols*i + j) + 1] - mean_s;*/
			//printf("i = %d, j = %d\n", i, j);
			pixel = sample.at<Vec2f>(i, j);
			//printf("pixel done, now P\n");
			P.at<float>(i, 0) = pixel[0] - mean_h;
			P.at<float>(i, 1) = pixel[1] - mean_s;
			
			//printf("done!\n");
			//P.at<float>(i,0) = 1.0f;
			//P.at<float>(i,1) = 1.0f;

			//P.data[i*j + 0] = sample.data[s_channels * (s_cols*i + j)] - mean_h - 0.5;
			//P.data[i*j + 1] = sample.data[s_channels * (s_cols*i + j) + 1] - mean_s;
		}
	}
	printf("C calc\n");
	Mat C = Mat::zeros(P.size(), CV_32F);
	C = P.t() * P; // P.t() *
	//printf("done\n");

	float detC = determinant(C);

	printf("C =\t%f %f\n\t%f %f\n", C.at<float>(0,0), C.at<float>(0,1), C.at<float>(1,0), C.at<float>(1,1));

	Mat mean(Size(2,1), CV_32F);
	mean.data[0] = mean_h;
	mean.data[1] = mean_s;

	probab = Mat::zeros(frame.size(), CV_32F);
	printf("main loop\n");
    do {
        capture >> frame;
        flip(frame, frame, 1);
		cvtColor(frame, effects, CV_BGR2HSV);
		
		effects.convertTo(effects, CV_32FC2);
	
		printf("probab mask start\n");
		calcProbabMask(&effects, mean, &C, detC, &probab);	
		probab /= *std::max_element(probab.begin<float>(), probab.end<float>());
		printf("probab mask calculated\n");


		//probab.convertTo(probab, CV_8UC3);
		imshow("Effects", probab); // effects
        imshow("CAM", frame); 

        key = waitKey(1);
    } while (key != KEY_ESCAPE);

    return 0;
}
