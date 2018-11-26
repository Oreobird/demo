#ifndef __EVG_DETECTOR_H__
#define __EVG_DETECTOR_H__

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>

#include <iomanip>

#include "superviseddescent/superviseddescent.hpp"
#include "superviseddescent/regressors.hpp"

#include "rcr/model.hpp"
#include "rcr/landmark.hpp"
#include "rcr/helpers.hpp"

#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"

#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include "cv.h"
#include "highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "face_detection.h"
#include "face_alignment.h"

using namespace std;
using namespace cv;

typedef enum
{
	DLIB = 0,
	SDM,
	SEETA,
} mark_detector_type_t;

typedef struct landmark
{
    cv::Point left_eye_outer;
    cv::Point right_eye_outer;
    cv::Point left_eye;
    cv::Point right_eye;
    cv::Point nose;
    cv::Point mouth_left;
    cv::Point mouth_right;
    cv::Point chin;

    cv::Rect face_box;

    cv::Rect emotion_box;
} landmark_t;


class ILandmarkDetector
{
	public:
		virtual int detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces,
							vector<landmark_t> *mark) = 0;
		virtual void draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path) = 0;
};

class SDMLandmarkDetector:public ILandmarkDetector
{
	public:
		SDMLandmarkDetector(){};
        SDMLandmarkDetector(std::string model_dir);
		int detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark);
		void draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path);

	private:
		std::vector<rcr::LandmarkCollection<cv::Vec2f>> m_landmarks;
		std::vector<cv::Rect> m_face_boxes;
        rcr::detection_model m_rcr_model;
};


class SeetaLandmarkDetector:public ILandmarkDetector
{
	public:
		SeetaLandmarkDetector(){};
        SeetaLandmarkDetector(std::string model_dir);
		int detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark);
		void draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path);

	private:
		vector<landmark_t> *m_mark;
        seeta::FaceAlignment m_point_detector;
};

class DlibLandmarkDetector:public ILandmarkDetector
{
	public:
		DlibLandmarkDetector(){};
        DlibLandmarkDetector(std::string model_dir);
		int detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark);
		void draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path);

	private:
		std::vector<dlib::full_object_detection> m_shapes;
		std::vector<cv::Rect> m_face_boxes;
        dlib::shape_predictor m_pose_model;
};


class Detector
{
	public:
		Detector(ILandmarkDetector *landmark_detector, std::string model_dir);
        ~Detector();
		int face_detect(cv::Mat img);
		void set_landmark_detector(ILandmarkDetector *landmark_detector);
		int landmark_detect(cv::Mat img, std::vector<landmark_t> *mark);
		void landmark_draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path);

	private:
        seeta::FaceDetection *m_face_detector;
		ILandmarkDetector *m_landmark_detector;
		std::vector<seeta::FaceInfo> m_faces;
};


#endif
