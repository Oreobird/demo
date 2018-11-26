#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "evg_common.h"
#include "evg_emotion.h"

using namespace std;
using namespace cv;

EmotionRecognizer::EmotionRecognizer(std::string model_dir)
{
	std::string weights = model_dir + "fer_model.pb";
	std::string pbtxt = model_dir + "fer_model.pbtxt";

	m_net = cv::dnn::readNetFromTensorflow(weights, pbtxt);

	int type_num = sizeof(m_emo_str) / sizeof(m_emo_str[0]);
	for (int i = 0; i < type_num; i++)
	{
		m_emotion.insert(std::pair<std::string, float>(m_emo_str[i], 0.5));
	}
}

static void get_idx(Mat pred, int neutral_idx, int *idx)
{
	float first = 0.0;
	float second = 0.0;
	int first_idx;
	int second_idx;

	for (int i = 0; i < pred.cols; i++)
	{
		float item = pred.at<float>(i);
		if (item > first)
		{
			second = first;
			first = item;
			second_idx = first_idx;
			first_idx = i;
		}
		else
		{
			if (item > second)
			{
				second = item;
				second_idx = i;
			}
		}
	}

	if (first_idx != neutral_idx
		&& second_idx == neutral_idx
		&& fabs(pred.at<float>(first_idx) - pred.at<float>(second_idx)) < 0.1)
	{
		*idx = neutral_idx;
	}
}

int EmotionRecognizer::recognize(cv::Mat img, cv::Rect *emotion_box, int input_size, float *prob)
{
	Mat gray;
	cv::cvtColor(img, gray, CV_BGR2GRAY);

	Mat crop_img = gray(*emotion_box);
	Mat input_img;
	cv::resize(crop_img, input_img, cv::Size(input_size, input_size));
	//cv::imshow("input_img", input_img);
	//cv::waitKey(0);

	Mat inputBlob = cv::dnn::blobFromImage(input_img, 0.00390625f, Size(input_size, input_size), Scalar(), false,false);
	m_net.setInput(inputBlob, "conv2d_input");
	Mat pred = m_net.forward("dense_2/Softmax");

	for (int i = 0; i < sizeof(m_emo_str) / sizeof(m_emo_str[0]); i++)
	{
		m_emotion[m_emo_str[i]] = pred.at<float>(i);
	}

	*prob = pred.at<float>(6);

	cout << pred << endl;
	return 0;
}

void EmotionRecognizer::draw(cv::Mat img, cv::Point org, cv::Scalar color)
{
	int thickness = 2;
	char text[64] = {0};

	for (int i = 0; i < sizeof(m_emo_str) / sizeof(m_emo_str[0]); i++)
	{
		snprintf(text, sizeof(text), "%s: %.3f", m_emo_str[i].c_str(), m_emotion[m_emo_str[i]]);
		cv::putText(img, text, cv::Point(org.x, org.y + 30 * i), cv::FONT_HERSHEY_COMPLEX,
					0.8, color, thickness, LINE_8, false);
	}
}
