#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <dirent.h>
#include <stdlib.h>

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

void handle_img(Mat img, Detector *detector, Quality *quality, int lm_type, ofstream &fp, std::string name)
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
		#if 0
		Mat crop_img = img(mark[0].emotion_box);
		Mat input_img;
		cv::resize(crop_img, input_img, cv::Size(120, 120));
		char path[256] = {0};
		snprintf(path, sizeof(path), "../data/glass_face/%s", name.c_str());
		//cv::imshow("face", input_img);
		//cv::waitKey(0);
		cv::imwrite(path, input_img);
		}
		#else

		quality->cal_all_scores(img, &mark[0]);

		if (lm_type == DLIB)
		{
			float occ_prob;
			OcclusionDetector occ_detector(model_dir);
			occ_detector.detect(img, &(mark[0].emotion_box), 96, &occ_prob);
			quality->set("occlusion", occ_prob);
			//occ_detector.draw(img, cv::Point(10, 60), cv::Scalar(0, 255, 0));

			float emt_prob;
			EmotionRecognizer emt_recognizer(model_dir);
			emt_recognizer.recognize(img, &(mark[0].emotion_box), 48, &emt_prob);
			quality->set("emotion", emt_prob);
			//emt_recognizer.draw(img, cv::Point(10, 30), cv::Scalar(0, 0, 255));

			if (fp)
			{
				//char name[64] = {0};
				//snprintf(name, sizeof(name), "%d.jpg", i);

				fp << name <<","<<quality->at("size_score")<<","<<quality->at("position_score")<<","
					<<quality->at("yaw_score")<<","<<quality->at("pitch_score")<<","
					<<quality->at("roll_score")<<","<<quality->at("contrast_score")<<","
					<<quality->at("clarity_score")<<","<<quality->at("brightness_score")<<","
					<<quality->at("emotion")<<","<<quality->at("occlusion")<<","
					<<occ_detector.at("left-eye")<<","<<occ_detector.at("right-eye")<<","
					<<occ_detector.at("nose")<<","<<occ_detector.at("mouth")<<","
					<<occ_detector.at("chin")<<endl;
			}
		}

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
	#endif
}

typedef void (*file_fn)(Detector *detector, Quality *quality, std::string file_path, std::string name, ofstream &fp);

void walk_dir(Detector *detector, Quality *quality, std::string dir, ofstream &fp, file_fn cb)
{
	struct stat fs;
	lstat(dir.c_str(), &fs);

	if (!S_ISDIR(fs.st_mode))
	{
		cout << "dir is not a valid dir"<<endl;
		return;
	}

	struct dirent *filename;
	DIR *pdir = NULL;

	pdir = opendir(dir.c_str());
	if (!pdir)
	{
		cout << "can not open dir" << dir << endl;
		return;
	}

	cout << "Open dir:"<<dir<<endl;

	char tmp_file[256] = {0};

	while ((filename = readdir(pdir)) != NULL)
	{
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0)
		{
			continue;
		}

		snprintf(tmp_file, sizeof(tmp_file), "%s/%s", dir.c_str(), filename->d_name);
		struct stat fs;
		lstat(tmp_file, &fs);

		if (S_ISDIR(fs.st_mode))
		{
 			cout << filename->d_name << endl;
			walk_dir(detector, quality, tmp_file, fp, cb);
		}
 		else
 		{
 			printf("%s\n", tmp_file);
 			cb(detector, quality, tmp_file, filename->d_name, fp);
 		}
 	}

	closedir(pdir);
}

void detect_img(Detector *detector, Quality *quality, std::string file_path, std::string file_name, ofstream &fp)
{
	cv::Mat img = imread(file_path);
	if (img.empty())
	{
		fa_dbg("imread failed\n");
		return;
	}

	int mark_type = 0;

	switch (mark_type)
	{
		case SDM:
		{
			SDMLandmarkDetector *sdm_lm_detector = new SDMLandmarkDetector();
			Detector detector(sdm_lm_detector, model_dir);
			handle_img(img, &detector, quality, SDM, fp, file_name);
			break;
		}
		case SEETA:
		{
			SeetaLandmarkDetector *seeta_lm_detector = new SeetaLandmarkDetector();
			Detector detector(seeta_lm_detector, model_dir);
			handle_img(img, &detector, quality, SEETA, fp, file_name);
			break;
		}
		case DLIB:
		default:
		{
			handle_img(img, detector, quality, DLIB, fp, file_name);
			break;
		}
	}
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

	ofstream fp("../data/test.txt");
	if (!fp)
	{
		fa_dbg("fp create failed\n");
		return -1;
	}

	fp << "file,size,position,yaw,pitch,roll,constrast,clarity,bright,emotion,occlusion,left-eye,right-eye,nose,mouth,chin" << endl;

	DlibLandmarkDetector *dlib_lm_detector = new DlibLandmarkDetector(model_dir);
	Detector detector(dlib_lm_detector, model_dir);
	Quality quality;

	walk_dir(&detector, &quality, img_path, fp, detect_img);

	fp.close();

	return 0;
}
