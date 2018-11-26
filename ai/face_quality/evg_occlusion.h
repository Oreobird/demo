#ifndef __EVG_OCCLUSION_H__
#define __EVG_OCCLUSION_H__

#include <opencv2/core/core.hpp>
#include <iostream>
#include <map>

using namespace std;
using namespace cv;

class OcclusionDetector
{
public:
    OcclusionDetector();
	OcclusionDetector(std::string model_dir);
	int detect(cv::Mat img, cv::Rect *occlusion_box, int input_size, float *prob);
	void draw(cv::Mat img, cv::Point org, cv::Scalar color);
    float at(const std::string &name);
    void add(const std::string &name, float score);

private:
	cv::dnn::Net m_net;
    std::map<string, float> m_occlusion;

    const std::string m_occ_str[6] = {"normal", "left-eye", "right-eye", "nose", "mouth", "chin"};
    //const std::string m_occlusion[1] = {"occlusion"};
};

#endif
