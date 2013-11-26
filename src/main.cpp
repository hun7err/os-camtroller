#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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

/*
void calcMatArrayMean(cv::Mat* arr, unsigned int size, cv::Mat* mean)
{
	unsigned int height = arr[0].size().height, width = arr[0].size().width;
	*mean = Mat::zeros(arr[0].size(), CV_8UC3);
	
	unsigned unsigned char **data,
					*mean_data = (unsigned char*)mean->data;
	
	data = new unsigned char* [size];
	unsigned int i = 0;
	for(; i < size; i++)
	{
		data[i] = (unsigned char*)arr[i].data;
	}
	i = 0;

	int cn = mean->channels();

	for(; i < mean->rows; i++)
	{
		for(int j = 0; j < mean->cols; j++)
		{
			unsigned int hue = 0, saturation = 0, value = 0;
			for(int cur_elem = 0; cur_elem < size; cur_elem++)
			{
				// dla wiêkszej iloœci próbek tutaj jest segfault

				hue += data[cur_elem][arr[cur_elem].step[0]*i + arr[cur_elem].step[1]*j + 0];
				saturation += data[cur_elem][arr[cur_elem].step[0]*i + arr[cur_elem].step[1]*j + 1];
				value += data[cur_elem][arr[cur_elem].step[0]*i + arr[cur_elem].step[1]*j + 2];
			}
			hue /= size;
			saturation /= size;
			value /= size;
			//cout << "mean_data filling" << endl;
			mean_data[mean->step[0]*i + mean->step[1]*j + 0] = hue;
			mean_data[mean->step[0]*i + mean->step[1]*j + 1] = saturation;
			mean_data[mean->step[0]*i + mean->step[1]*j + 2] = value;
		}
	}
	//cout << "end" << endl;

	delete [] data;
}*/

void calcProbabilityMask(Mat image, Mat mean, Mat covar, Mat& mask)
{
	mask = Mat::zeros(image.size(), CV_8UC3);
	
	unsigned unsigned char *data, *mask_data;
	
	data = image.data;
	unsigned int i = 0;

	int cn = image.channels();

	for(; i < image.rows; i++)
	{
		for(int j = 0; j < image.cols; j++)
		{
			Scalar pixel(
							data[image.step[0]*i + image.step[1]*j + 0],
							data[image.step[0]*i + image.step[1]*j + 1]
						);
			
			double P = 1 / sqrt(DPISQRD * determinant(covar)) * exp( -0.5 * (pixel - mean) * covar.inv() * (pixel-mean).t());
		}
	}
	//cout << "end" << endl;

	delete [] data;
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

    namedWindow("CAM", CV_WINDOW_AUTOSIZE);

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
	const int sample_count = 8;
	Mat samples[sample_count];
	int cur_sample_id = 0;
	bool sample_gathering = false;

	do {
        capture >> frame;
        flip(frame, frame, 1);
		frame.copyTo(blurred, mask);
		GaussianBlur(blurred, blurred, Size(25, 25), 10, 10);
		blurred.copyTo(frame, mask);
		
		rectangle(frame, Point(rect_x,rect_y), Point(rect_x+rect_width+2*line_thickness, rect_y+rect_height+2*line_thickness), Scalar(0,0,255), line_thickness);
        imshow("CAM", frame);

        key = waitKey(1);
		if(key == KEY_SPACE)
		{
			sample_gathering = true;
		}
		if(sample_gathering)
		{
			samples[cur_sample_id] = Mat::zeros(Size(rect_width, rect_height), CV_8UC3);
			Mat(frame, Rect(rect_x+line_thickness, rect_y+line_thickness, rect_width, rect_height)).copyTo(samples[cur_sample_id]);
			//cvtColor(samples[cur_sample_id], samples[cur_sample_id], CV_BGR2HSV);
			cur_sample_id++;
		}
    } while (cur_sample_id < sample_count);
	 
	/*Mat sample = Mat::zeros(Size(rect_width, rect_height), CV_8UC3);
	Mat(frame, Rect(rect_x+line_thickness, rect_y+line_thickness, rect_width, rect_height)).copyTo(sample);
	cvtColor(sample, sample, CV_BGR2HSV);*/
	
    namedWindow("Effects", CV_WINDOW_AUTOSIZE);

	Mat s;
	//calcMatArrayMean(samples, sample_count, &s);
	Mat covar, mean;
	calcCovarMatrix(samples, sample_count, covar, mean, CV_COVAR_NORMAL);

    do {
        capture >> frame;
        flip(frame, frame, 1);
		cvtColor(frame, effects, CV_BGR2HSV); // CV_BGR2HSV
        split(effects, channels);
		equalizeHist(channels[2], channels[2]);
        effects = Mat::zeros(effects.size(), CV_8UC3);
        merge(channels, 3, effects);
        cvtColor(effects, effects, CV_HSV2BGR);

        imshow("CAM", frame);
        imshow("Effects", s); // effects 

        key = waitKey(1);
    } while (key != KEY_ESCAPE);

    return 0;
}
