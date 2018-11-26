#ifndef __QUALITY_H__
#define __QUALITY_H__

#include <opencv2/core/core.hpp>
#include <iostream>
#include <map>
#include "evg_detector.h"

using namespace std;
using namespace cv;

#define PI 3.141592
#define SIZE_THRESHOLD      0.050
#define ANGLE_THRESHOLD     0 //0.900
#define POS_THRESHOLD       0.800
#define BRIGHT_THRESHOLD    0.700
#define CLARITY_THRESHOLD   0.750
#define CONTRAST_THRESHOLD  0.700
#define QUALITY_THRESHOLD   60.00
#define YAW_THRESHOLD       20.0
#define PITCH_THRESHOLD     20.0
#define ROLL_THRESHOLD      20.0
#define EMOTION_THRESHOLD   0.400
#define OCCLUSION_THRESHOLD 0.500


class Quality
{
    public:
        Quality();
        ~Quality();
        float size_score(void);
        float position_score(void);
        float angle_score(void);
        float contrast_score(void);
        float brightness_score(void);
        float clarity_score(void);

        cv::Vec3d pose_score(void);

        float quality_score(void);
        bool is_pass(void);

        void cal_all_scores(Mat img, landmark_t *mark);
        void show_all_scores(void);

        float at(const std::string& name);
        void add(const std::string& name, float score);
        void set(const std::string& name, float score);

    private:
        Mat m_img;
        landmark_t *m_mark;

    private:
        float m_total;
        float m_size;
        float m_pos;
        float m_angle;
        float m_contrast;
        float m_brightness;
        float m_clarity;
        cv::Vec3d m_pose_param;


        std::map<string, float> m_scores;
};

#endif
