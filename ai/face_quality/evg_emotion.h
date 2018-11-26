#ifndef __EVG_EMOTION_H__
#define __EVG_EMOTION_H__

#include <opencv2/core/core.hpp>
#include <iostream>
#include <map>

using namespace std;
using namespace cv;

class EmotionRecognizer
{
	public:
        EmotionRecognizer();
		EmotionRecognizer(std::string model_dir);
		int recognize(cv::Mat img, cv::Rect *emotion_box, int input_size, float *prob);
		void draw(cv::Mat img, cv::Point org, cv::Scalar color);

	private:
        cv::dnn::Net m_net;
        std::map<string, float> m_emotion;
        const std::string m_emo_str[7] = {"anger", "disgust", "fear", "happy", "sad", "surprise", "neutral"};
};

#endif
