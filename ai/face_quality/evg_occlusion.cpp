#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "evg_common.h"
#include "evg_occlusion.h"

using namespace std;
using namespace cv;

OcclusionDetector::OcclusionDetector(std::string model_dir)
{
	std::string weights = model_dir + "cofw_model.pb";
	std::string pbtxt = model_dir + "cofw_model.pbtxt";

	m_net = cv::dnn::readNetFromTensorflow(weights, pbtxt);

	int type_num = sizeof(m_occ_str) / sizeof(m_occ_str[0]);
	for (int i = 0; i < type_num; i++)
	{
		m_occlusion.insert(std::pair<std::string, float>(m_occ_str[i], 0.5));
	}
}

int OcclusionDetector::detect(cv::Mat img, cv::Rect *occlusion_box, int input_size, float *prob)
{
	Mat gray;
	cv::cvtColor(img, gray, CV_BGR2GRAY);

	Mat crop_img = gray(*occlusion_box);
	Mat input_img;
	cv::resize(crop_img, input_img, cv::Size(input_size, input_size));
	//cv::imshow("input_img", input_img);
	//cv::waitKey(0);

	Mat inputBlob = cv::dnn::blobFromImage(input_img, 0.00390625f, Size(input_size, input_size), Scalar(), false,false);
	m_net.setInput(inputBlob, "conv2d_input");
	Mat pred = m_net.forward("dense_2/Sigmoid");

	for (int i = 0; i < sizeof(m_occ_str) / sizeof(m_occ_str[0]); i++)
	{
		m_occlusion[m_occ_str[i]] = pred.at<float>(i);
	}

	*prob = pred.at<float>(0);

	cout << pred << endl;
}

void OcclusionDetector::draw(cv::Mat img, cv::Point org, cv::Scalar color)
{
	int thickness = 2;
	char text[64] = {0};

	for (int i = 0; i < sizeof(m_occ_str) / sizeof(m_occ_str[0]); i++)
	{
		snprintf(text, sizeof(text), "%s: %.3f", m_occ_str[i].c_str(), m_occlusion[m_occ_str[i]]);
		cv::putText(img, text, cv::Point(org.x, org.y + 30 * i), cv::FONT_HERSHEY_COMPLEX,
					0.8, color, thickness, LINE_8, false);
	}
}

float OcclusionDetector::at(const std::string &name)
{
    return m_occlusion.at(name);
}

void OcclusionDetector::add(const std::string &name, float score)
{
    m_occlusion.insert(std::pair<std::string, float>(name, score));
}

