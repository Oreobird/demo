#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

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
#include "evg_msg.h"

using namespace std;
using namespace cv;
using namespace boost::interprocess;

std::string model_dir = "/model/";
std::string json_path;
std::string img_path;
std::string result_img_path;

static void handle_img(Mat img, Detector *detector,
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
		quality->set("occlusion", occ_prob);
		//occ_detector.draw(img, cv::Point(10, 60), cv::Scalar(0, 255, 0));

		float emt_prob;
		emt_recognizer->recognize(img, &(mark[0].emotion_box), 48, &emt_prob);
		quality->set("emotion", emt_prob);
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


	DlibLandmarkDetector *dlib_lm_detector = new DlibLandmarkDetector(model_dir);
	Detector detector(dlib_lm_detector, model_dir);
	OcclusionDetector occ_detector(model_dir);
	EmotionRecognizer emt_recognizer(model_dir);
	Quality quality;

	evg_msg_t msg;
	message_queue::remove("aivideo_srv_queue");
	message_queue srv_mq(open_or_create, "aivideo_srv_queue", 10, sizeof(msg));
	message_queue::remove("aivideo_cli_queue");
	message_queue cli_mq(open_or_create, "aivideo_cli_queue", 10, sizeof(msg));

	message_queue::size_type recv_size;
	unsigned int priority;

	while (true)
	{
		memset(&msg, 0, sizeof(msg));
		srv_mq.receive(&msg, sizeof(msg), recv_size, priority);

		fa_dbg("recv_size=%d, msg.len=%d\n", (int)recv_size, msg.len);
		fa_dbg("msg: %s\n", msg.payload);

		Json::Reader reader;
		Json::Value root;
		if (reader.parse(msg.payload, root))
		{
			img_path = root["img_path"].asString();
			json_path = root["json_path"].asString();
			result_img_path = root["result_img_path"].asString();

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

			cout << "img_path:" << img_path << endl;
			cout << "json_path:" << json_path << endl;
			cout << "result_img_path:" << result_img_path << endl;

			cv::Mat img = imread(img_path);
			if (img.empty())
			{
				fa_dbg("imread failed\n");
			}
			else
			{
				handle_img(img, &detector, &occ_detector, &emt_recognizer, &quality);
			}

			Json::FastWriter fw;
			Json::Value root;
			root["err_code"] = 0;
			std::string json_str = fw.write(root);

			memset(&msg, 0, sizeof(msg));
			snprintf(msg.payload, sizeof(msg.payload), "%s", json_str.c_str());
			msg.len = json_str.length();

			cli_mq.send(&msg, sizeof(msg), 0);
		}
	}

	return 0;
}
