#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <jsoncpp/json/json.h>

#include <iomanip>

#include "evg_common.h"
#include "evg_json.h"

using namespace std;

EvgJson::EvgJson(std::string json_path)
{
	m_json_path = json_path;
}

int EvgJson::gen_exception_json(int face_num)
{
  Json::Value root;
  Json::FastWriter fw;

  if (face_num == 0)
  {
    Json::Value code;
    root["err_code"] = Json::Value(-1);
    root["msg"] = Json::Value("No face detected!");
  }
  else if (face_num > 1)
  {
    root["err_code"] = Json::Value(-2);
    root["msg"] = Json::Value("More than 1 face, not supported yet.");
  }

  ofstream json_file(m_json_path);
  if (!json_file)
  {
    fa_dbg("json_file open failed.\n");
    return -1;
  }

  json_file << fw.write(root);
  json_file.close();
  return 0;
}

int EvgJson::gen_face_json(landmark_t *mark, Quality *qof)
{
    Json::Value root;
    Json::FastWriter fw;

    root["err_code"] = Json::Value(0);
    root["pass"] = Json::Value(qof->is_pass());
    //quality result
    Json::Value quality;
    char score_str[128] = {0};
	float score;

	score = qof->at("size_score");
    snprintf(score_str, sizeof(score_str), "%.3f", score);
    Json::Value size_item;
	size_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", SIZE_THRESHOLD);
	size_item["threshold"] = Json::Value(score_str);
	size_item["pass"] = Json::Value(score >= SIZE_THRESHOLD ? true : false);
    quality["size"] = size_item;

	score = qof->at("position_score");
	Json::Value position_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	position_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", POS_THRESHOLD);
	position_item["threshold"] = Json::Value(score_str);
	position_item["pass"] = Json::Value(score >= POS_THRESHOLD ? true : false);
    quality["pos"] = position_item;

	score = qof->at("yaw_score");
	Json::Value yaw_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	yaw_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", YAW_THRESHOLD);
	yaw_item["threshold"] = Json::Value(score_str);
	yaw_item["pass"] = Json::Value(score <= YAW_THRESHOLD ? true : false);
	quality["yaw"] = yaw_item;

	score = qof->at("pitch_score");
	Json::Value pitch_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	pitch_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", PITCH_THRESHOLD);
	pitch_item["threshold"] = Json::Value(score_str);
	pitch_item["pass"] = Json::Value(score <= PITCH_THRESHOLD ? true : false);
	quality["pitch"] = pitch_item;

	score = qof->at("roll_score");
	Json::Value roll_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	roll_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", ROLL_THRESHOLD);
	roll_item["threshold"] = Json::Value(score_str);
	roll_item["pass"] = Json::Value(score <= ROLL_THRESHOLD ? true : false);
	quality["roll"] = roll_item;

    score = qof->at("contrast_score");
    Json::Value contrast_item;
    snprintf(score_str, sizeof(score_str), "%.3f", score);
	contrast_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", CONTRAST_THRESHOLD);
	contrast_item["threshold"] = Json::Value(score_str);
	contrast_item["pass"] = Json::Value(score >= CONTRAST_THRESHOLD ? true : false);
    quality["contrast"] = contrast_item;

	score = qof->at("clarity_score");
	Json::Value clarity_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	clarity_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", CLARITY_THRESHOLD);
	clarity_item["threshold"] = Json::Value(score_str);
	clarity_item["pass"] = Json::Value(score >= CLARITY_THRESHOLD ? true : false);
    quality["clarity"] = clarity_item;

    score = qof->at("brightness_score");
    Json::Value brightness_item;
    snprintf(score_str, sizeof(score_str), "%.3f", score);
	brightness_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", BRIGHT_THRESHOLD);
	brightness_item["threshold"] = Json::Value(score_str);
	brightness_item["pass"] = Json::Value(score >= BRIGHT_THRESHOLD ? true : false);
    quality["brightness"] = brightness_item;

	score = qof->at("emotion");
	Json::Value emotion_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	emotion_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", EMOTION_THRESHOLD);
	emotion_item["threshold"] = Json::Value(score_str);
	emotion_item["pass"] = Json::Value(score >= EMOTION_THRESHOLD ? true : false);
    quality["emotion"] = emotion_item;

	score = qof->at("occlusion");
	Json::Value occ_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	occ_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", OCCLUSION_THRESHOLD);
	occ_item["threshold"] = Json::Value(score_str);
	occ_item["pass"] = Json::Value(score >= OCCLUSION_THRESHOLD ? true : false);
    quality["occlusion"] = occ_item;

	score = qof->at("quality_score");
	if((!qof->is_pass())&&(score >= 60))
	{
		score = 59;
	}
	Json::Value quality_item;
	snprintf(score_str, sizeof(score_str), "%.3f", score);
	quality_item["value"] = Json::Value(score_str);
    snprintf(score_str, sizeof(score_str), "%.3f", QUALITY_THRESHOLD);
	quality_item["threshold"] = Json::Value(score_str);
	//quality_item["pass"] = Json::Value(score >= QUALITY_THRESHOLD ? true : false);
	quality_item["pass"] = Json::Value(qof->is_pass());
    quality["total"] = quality_item;
    root["quality"] = quality;

    //landmark result
    Json::Value box;
    box["x"] = Json::Value(mark->face_box.x);
    box["y"] = Json::Value(mark->face_box.y);
    box["width"] = Json::Value(mark->face_box.width);
    box["height"] = Json::Value(mark->face_box.height);

    root["box"] = box;

    Json::Value left_eye;
    left_eye["x"] = Json::Value(mark->left_eye.x);
    left_eye["y"] = Json::Value(mark->left_eye.y);
    root["left_eye"] = left_eye;

    Json::Value right_eye;
    right_eye["x"] = Json::Value(mark->right_eye.x);
    right_eye["y"] = Json::Value(mark->right_eye.y);
    root["right_eye"] = right_eye;

    Json::Value nose;
    nose["x"] = Json::Value(mark->nose.x);
    nose["y"] = Json::Value(mark->nose.y);
    root["nose"] = nose;

    Json::Value mouth_left;
    mouth_left["x"] = Json::Value(mark->mouth_left.x);
    mouth_left["y"] = Json::Value(mark->mouth_left.y);
    root["mouth_left"] = mouth_left;

    Json::Value mouth_right;
    mouth_right["x"] = Json::Value(mark->mouth_right.x);
    mouth_right["y"] = Json::Value(mark->mouth_right.y);
    root["mouth_right"] = mouth_right;

    ofstream json_file(m_json_path);
    if (!json_file)
    {
        return -1;
    }

    json_file << fw.write(root);
    json_file.close();

    return 0;
}
