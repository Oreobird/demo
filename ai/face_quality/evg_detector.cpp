#include "evg_common.h"
#include "evg_quality.h"
#include "evg_timing.h"

#include "evg_detector.h"

using namespace std;
using namespace cv;

void SDMLandmarkDetector::draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path)
{
	int face_num = m_face_boxes.size();
	for (int i = 0; i < face_num; i++)
	{
    	rectangle(img, m_face_boxes[i], color, 4);
    	rcr::draw_landmarks(img, m_landmarks[i], color);
    }

	cv::imwrite(img_save_path, img);

	if (show)
	{
    	cv::imshow("seeta_landmark", img);
    	cv::waitKey(0);
    }
}

void sdm_landmarks_to_mark(rcr::LandmarkCollection<cv::Vec2f> landmarks, landmark_t *mark)
{
	cv::Mat lm_mat = rcr::to_row(landmarks);
	int num_landmarks = std::max(lm_mat.cols, lm_mat.rows) / 2;

	mark->left_eye.x = (lm_mat.at<float>(13) + lm_mat.at<float>(10)) / 2;
	mark->left_eye.y = (lm_mat.at<float>(13 + num_landmarks) + lm_mat.at<float>(10 + num_landmarks)) / 2;
	mark->left_eye_outer.x = lm_mat.at<float>(13);
    mark->left_eye_outer.y = lm_mat.at<float>(13 + num_landmarks);

	mark->right_eye.x = (lm_mat.at<float>(4) + lm_mat.at<float>(7)) / 2;
	mark->right_eye.y = (lm_mat.at<float>(4 + num_landmarks) + lm_mat.at<float>(7+ num_landmarks)) / 2;
	mark->right_eye_outer.x = lm_mat.at<float>(4);
    mark->right_eye_outer.y = lm_mat.at<float>(4 + num_landmarks);

	mark->nose.x = lm_mat.at<float>(1);
    mark->nose.y = lm_mat.at<float>(1 + num_landmarks);

	mark->mouth_left.x = lm_mat.at<float>(18);
    mark->mouth_left.y = lm_mat.at<float>(18 + num_landmarks);
	mark->mouth_right.x = lm_mat.at<float>(16);
    mark->mouth_right.y = lm_mat.at<float>(16 + num_landmarks);

	mark->chin.x = lm_mat.at<float>(0);
    mark->chin.y = lm_mat.at<float>(0 + num_landmarks);
}


SDMLandmarkDetector::SDMLandmarkDetector(std::string model_dir)
{
	m_rcr_model = rcr::load_detection_model((model_dir + "face_landmarks_model_rcr_22.bin").c_str());
}

int SDMLandmarkDetector::detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark)
{
    landmark_t mark_item;
    m_face_boxes.clear();
    m_landmarks.clear();

	for (int i = 0; i < faces->size(); i++)
	{
		cv::Rect face(faces->at(i).bbox.x, faces->at(i).bbox.y,
					  faces->at(i).bbox.width, faces->at(i).bbox.height);
		auto landmarks = m_rcr_model.detect(img, face);
		m_face_boxes.push_back(face);
		m_landmarks.push_back(landmarks);
		mark_item.face_box = cv::Rect(faces->at(i).bbox.x, faces->at(i).bbox.y,
								        faces->at(i).bbox.width, faces->at(i).bbox.height);
		sdm_landmarks_to_mark(landmarks, &mark_item);
		mark->push_back(mark_item);
	}

	return 0;
}


void SeetaLandmarkDetector::draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path)
{
	std::vector<landmark_t>::iterator it;

	for (it = m_mark->begin(); it != m_mark->end(); it++)
	{
	    cv::rectangle(img, it->face_box, color, 4);

	    cv::circle(img, it->left_eye, 4, color, -1);
	    cv::circle(img, it->right_eye, 4, color, -1);
	    cv::circle(img, it->nose, 4, color, -1);
	    cv::circle(img, it->mouth_left, 4, color, -1);
	    cv::circle(img, it->mouth_right, 4, color, -1);
	}

    cv::imwrite(img_save_path, img);

	if (show)
	{
    	cv::imshow("seeta_landmark", img);
    	cv::waitKey(0);
    }
}

void points_to_mark(seeta::FaceInfo face, seeta::FacialLandmark points[], landmark_t *mark)
{
	mark->face_box = cv::Rect(face.bbox.x, face.bbox.y,
								face.bbox.width, face.bbox.height);

	mark->left_eye = cv::Point(points[0].x + 0.1f, points[0].y + 0.1f);
	mark->right_eye = cv::Point(points[1].x + 0.1f, points[1].y + 0.1f);
	mark->nose = cv::Point(points[2].x + 0.1f, points[2].y + 0.1f);
	mark->mouth_left = cv::Point(points[3].x + 0.1f, points[3].y + 0.1f);
	mark->mouth_right = cv::Point(points[4].x + 0.1f, points[4].y + 0.1f);
}

SeetaLandmarkDetector::SeetaLandmarkDetector(std::string model_dir)
{
	seeta::FaceAlignment m_point_detector((model_dir + "seeta_fa_v1.1.bin").c_str());
}

int SeetaLandmarkDetector::detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark)
{
	cv::Mat gray;

	if (img.channels() == 3)
	{
		cv::cvtColor(img, gray, CV_BGR2GRAY);
	}
	else
	{
		gray = img;
	}

	seeta::ImageData st_img;
	st_img.data = gray.data;
	st_img.width = gray.cols;
	st_img.height = gray.rows;
	st_img.num_channels = 1;

    landmark_t mark_item;
    seeta::FacialLandmark points[5];

    for (int i = 0; i < faces->size(); i++)
    {
	    m_point_detector.PointDetectLandmarks(st_img, faces->at(i), points);
	    points_to_mark(faces->at(i), points, &mark_item);
	    mark->push_back(mark_item);
        m_mark = mark;
    }

    return 0;
}

void DlibLandmarkDetector::draw(cv::Mat img, cv::Scalar color, bool show, std::string save_img_path)
{
	int face_num = m_face_boxes.size();
	for (int i = 0; i < face_num; i++)
	{
	    cv::rectangle(img, m_face_boxes[i], color, 4);

		for (int j = 0; j < 68; j++)
		{
			cv::circle(img, cv::Point(m_shapes[i].part(j).x(), m_shapes[i].part(j).y()), 4, color, -1);
		}
	}

    cv::imwrite(save_img_path, img);

	if (show)
	{
    	cv::imshow("seeta_landmark", img);
    	cv::waitKey(0);
    }

}

void dlib_config_emotion_mark(cv::Mat img, dlib::full_object_detection shape, landmark_t *mark)
{
	int point_num = shape.num_parts();
	int max_x = 0, max_y = 0, min_x = 65535, min_y = 65535;
	for (int i = 0; i < point_num; i++)
	{
		if (shape.part(i).x() > max_x)
		{
			max_x = shape.part(i).x();
		}
		if (shape.part(i).x() < min_x)
		{
			min_x = shape.part(i).x();
		}
		if (shape.part(i).y() > max_y)
		{
			max_y = shape.part(i).y();
		}
		if (shape.part(i).y() < min_y)
		{
			min_y = shape.part(i).y();
		}
	}

	float Lx = float(abs(max_x - min_x));
	float Ly = float(abs(max_y - min_y));
	float Lmax = (Lx > Ly ? Lx : Ly)*1.05;
	int delta = int(Lmax / 2);
	int center_x = int((max_x + min_x)/2);
	int center_y = int((max_y + min_y)/2);
	int start_x = center_x - delta;
	int start_y = center_y - delta;
	int end_x = center_x + delta;
	int end_y = center_y + delta;

	start_x = start_x < 0 ? 0 : start_x;
	start_y = start_y < 0 ? 0 : start_y;
	end_x = end_x > img.cols ? img.cols : end_x;
	end_y = end_y > img.rows ? img.rows : end_y;

	mark->emotion_box = cv::Rect(start_x, start_y, end_x - start_x, end_y - start_y);
}


void dlib_landmarks_to_mark(cv::Mat img, dlib::full_object_detection shape, landmark_t *mark)
{
	mark->left_eye.x = (shape.part(36).x() + shape.part(39).x()) / 2;
	mark->left_eye.y = (shape.part(37).y() + shape.part(41).y()) / 2;
	mark->left_eye_outer.x = shape.part(36).x();
    mark->left_eye_outer.y = shape.part(36).y();

	mark->right_eye.x = (shape.part(42).x() + shape.part(45).x()) / 2;
	mark->right_eye.y = (shape.part(43).y() + shape.part(47).y()) / 2;
	mark->right_eye_outer.x = shape.part(45).x();
    mark->right_eye_outer.y = shape.part(45).y();

	mark->nose.x = shape.part(30).x();
    mark->nose.y = shape.part(30).y();

	mark->mouth_left.x = shape.part(48).x();
    mark->mouth_left.y = shape.part(48).y();
	mark->mouth_right.x = shape.part(54).x();
    mark->mouth_right.y = shape.part(54).y();

	mark->chin.x = shape.part(8).x();
    mark->chin.y = shape.part(8).y();
	//printf("chin:(%d, %d)\n", mark->chin.x, mark->chin.y);

	dlib_config_emotion_mark(img, shape, mark);
}

DlibLandmarkDetector::DlibLandmarkDetector(std::string model_dir)
{
	dlib::deserialize((model_dir + "shape_predictor_68_face_landmarks.dat").c_str()) >> m_pose_model;
}

int DlibLandmarkDetector::detect(cv::Mat img, std::vector<seeta::FaceInfo> *faces, vector<landmark_t> *mark)
{
	dlib::cv_image<dlib::bgr_pixel> cimg(img);
	m_face_boxes.clear();
	m_shapes.clear();

	landmark_t mark_item;
	for (int i = 0; i < faces->size(); i++)
	{
		cv::Rect cvface(faces->at(i).bbox.x, faces->at(i).bbox.y,
					  faces->at(i).bbox.width, faces->at(i).bbox.height);
		m_face_boxes.push_back(cvface);

		fa_dbg("face:(%d, %d, %d, %d)\n", faces->at(i).bbox.x, faces->at(i).bbox.y,
					  		faces->at(i).bbox.x + faces->at(i).bbox.width,
					  		faces->at(i).bbox.y + faces->at(i).bbox.height);

		dlib::rectangle face(faces->at(i).bbox.x, faces->at(i).bbox.y,
					  		faces->at(i).bbox.x + faces->at(i).bbox.width,
					  		faces->at(i).bbox.y + faces->at(i).bbox.height);

		dlib::full_object_detection shape = m_pose_model(cimg, face);
		m_shapes.push_back(shape);

		mark_item.face_box = cv::Rect(faces->at(i).bbox.x, faces->at(i).bbox.y,
								    faces->at(i).bbox.width, faces->at(i).bbox.height);
		dlib_landmarks_to_mark(img, shape, &mark_item);

		mark->push_back(mark_item);
	}

	#ifdef CAL_TIME
	double pose_time = calcElapsed(start, now());
	printf("pose_time: %d ms.\n", (int)(pose_time * 1000));
	#endif

	return 0;
}

void Detector::set_landmark_detector(ILandmarkDetector * landmark_detector)
{
	m_landmark_detector = landmark_detector;
}
int Detector::landmark_detect(cv::Mat img, std::vector<landmark_t> *mark)
{
	return m_landmark_detector->detect(img, &m_faces, mark);
}
void Detector::landmark_draw(cv::Mat img, cv::Scalar color, bool show, std::string img_save_path)
{
	m_landmark_detector->draw(img, color, show, img_save_path);
}

Detector::Detector(ILandmarkDetector *landmark_detector, std::string model_dir)
{
	m_face_detector = new seeta::FaceDetection((model_dir + "seeta_fd_frontal_v1.0.bin").c_str());
	m_face_detector->SetMinFaceSize(40);
	m_face_detector->SetScoreThresh(2.f);
	m_face_detector->SetImagePyramidScaleFactor(0.8f);
	m_face_detector->SetWindowStep(4, 4);

	m_landmark_detector = landmark_detector;
}

Detector::~Detector()
{
	if (m_face_detector)
	{
		delete m_face_detector;
	}

	if (m_landmark_detector)
	{
		delete m_landmark_detector;
	}
}

int Detector::face_detect(cv::Mat img)
{
	cv::Mat gray;

	if (img.channels() == 3)
	{
		cv::cvtColor(img, gray, CV_BGR2GRAY);
	}
	else
	{
		gray = img;
	}

	seeta::ImageData st_img;
	st_img.data = gray.data;
	st_img.width = gray.cols;
	st_img.height = gray.rows;
	st_img.num_channels = 1;

	m_faces = m_face_detector->Detect(st_img);
	int32_t face_num = static_cast<int32_t>(m_faces.size());

	std::vector<seeta::FaceInfo>::iterator it;
	for (it = m_faces.begin(); it != m_faces.end(); it++)
	{
		if ((*it).bbox.x < 0)
		{
			(*it).bbox.x = 0;
		}
		if ((*it).bbox.y < 0)
		{
			(*it).bbox.y = 0;
		}
		if ((*it).bbox.x + (*it).bbox.width > gray.cols)
		{
			(*it).bbox.width = gray.cols - (*it).bbox.x;
		}
		if ((*it).bbox.y + (*it).bbox.height > gray.rows)
		{
			(*it).bbox.height = gray.rows - (*it).bbox.y;
		}
	}

	return face_num;
}
