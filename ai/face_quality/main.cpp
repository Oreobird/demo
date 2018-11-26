#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>

#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"

#include "cv.h"
#include "highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "evg_common.h"
#include "evg_quality.h"
#include "evg_timing.h"
#include "evg_emotion.h"
#include "evg_occlusion.h"
#include "evg_detector.h"
#include "evg_json.h"

using namespace std;
using namespace cv;

std::string model_dir = "/model/";
std::string json_path;
std::string img_path;
std::string result_img_path;

void usage(char *bin_name)
{
    printf("usage: %s -i image_file -j json_file -o result.jpg -t type\n", bin_name);
   	printf("	-i: image file path to detect.\n");
   	printf("	-j: output json file path.\n");
   	printf("	-o: output detect result image file path.\n");
   	printf("	-t: landmark detect type, int [0, 2]\n");
    printf("eg: %s -i ./001.jpg -j ./quality.json -o ./result.jpg -t 0\n", bin_name);
}

void handle_img(Mat img, Detector *detector,
				OcclusionDetector *occ_detector, EmotionRecognizer *emt_recognizer,
				Quality *quality)
{
	double start = now();
	double detect_time;
	double landmark_time;
	double quality_time;
	double draw_save_time;

	std::vector<landmark_t> mark;

	int face_num = detector->face_detect(img);

	#ifdef CAL_TIME
	detect_time = calcElapsed(start, now());
	printf("detect_time: %d ms.\n", (int)(detect_time * 1000));
	start = now();
	#endif

	detector->landmark_detect(img, &mark);

	#ifdef CAL_TIME
	landmark_time = calcElapsed(start, now());
	printf("landmark_time: %d ms.\n", (int)(landmark_time * 1000));
	start = now();
	#endif

	EvgJson evg_json(json_path);

	if (face_num == 1)
	{
		quality->cal_all_scores(img, &mark[0]);

		float occ_prob;
		occ_detector->detect(img, &(mark[0].emotion_box), 96, &occ_prob);
		quality->add("occlusion", occ_prob);
		//occ_detector.draw(img, cv::Point(10, 60), cv::Scalar(0, 255, 0));

		float emt_prob;
		emt_recognizer->recognize(img, &(mark[0].emotion_box), 48, &emt_prob);
		quality->add("emotion", emt_prob);
		//emt_recognizer.draw(img, cv::Point(10, 30), cv::Scalar(0, 0, 255));

		evg_json.gen_face_json(&mark[0], quality);
	}
	else
	{
		evg_json.gen_exception_json(face_num);
	}

	#ifdef CAL_TIME
	quality_time = calcElapsed(start, now());
	printf("quality_time: %d ms.\n", (int)(quality_time * 1000));
	start = now();
	#endif

	detector->landmark_draw(img, cv::Scalar(0, 0, 255), false, result_img_path);

	#ifdef CAL_TIME
	draw_save_time = calcElapsed(start, now());
	printf("draw_save_time: %d ms.\n", (int)(draw_save_time * 1000));
	#endif
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char default_img[256] = {0};
	char default_json[256] = {0};
	char default_result[256] = {0};
	char ch;
	int mark_type = 0;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);
	model_dir = prog_path + model_dir;

	while ((ch = getopt(argc, argv, "hj:i:o:t:")) != EOF)
	{
		switch (ch)
		{
			case 'j':
				json_path = optarg;
				break;
			case 'i':
				img_path = optarg;
				break;
			case 'o':
				result_img_path = optarg;
				break;
			case 't':
				mark_type = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_path.empty())
	{
		snprintf(default_img, sizeof(default_img), "%s/sample.jpg", prog_path);
		img_path = default_img;
	}
	if (json_path.empty())
	{
		snprintf(default_json, sizeof(default_json), "%s/quality.json", prog_path);
		json_path = default_json;
	}
	if (result_img_path.empty())
	{
		snprintf(default_result, sizeof(default_result), "%s/result.jpg", prog_path);
		result_img_path = default_result;
	}


	DlibLandmarkDetector *dlib_lm_detector = new DlibLandmarkDetector(model_dir);
	Detector detector(dlib_lm_detector, model_dir);
	OcclusionDetector occ_detector(model_dir);
	EmotionRecognizer emt_recognizer(model_dir);
	Quality quality;


	cv::Mat img = imread(img_path);
	if (img.empty())
	{
		fa_dbg("imread failed\n");
		return -1;
	}

	handle_img(img, &detector, &occ_detector, &emt_recognizer, &quality);

	return 0;
}
