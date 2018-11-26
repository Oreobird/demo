#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <Python.h>

#include "evg_quality.h"

using namespace std;
using namespace cv;

static void init_python()
{
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        return;
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import sklearn");
    PyRun_SimpleString("print ('---import sys---')");
    PyRun_SimpleString("sys.path.append('./')");
}

static void exit_python()
{
	Py_Finalize();
}

Quality::Quality()
{
	init_python();

	add("size_score", 0);
	add("position_score", 0);
	add("angle_score", 0);
	add("yaw_score", 0);
	add("pitch_score", 0);
	add("roll_score", 0);
	add("contrast_score", 0);
	add("brightness_score", 0);
	add("clarity_score", 0);
	add("quality_score", 0);
	add("occlusion", 0);
	add("emotion", 0);
}

Quality::~Quality()
{
	exit_python();
}

float Quality::size_score(void)
{
	if ((m_mark->left_eye.x < m_mark->face_box.x)
		|| (m_mark->right_eye.x > (m_mark->face_box.x + m_mark->face_box.width)))
	{
		return 0;
	}

	if ((m_mark->mouth_left.x < m_mark->face_box.x)
		|| (m_mark->mouth_right.x > (m_mark->face_box.x + m_mark->face_box.width)))
	{
		return 0;
	}

	if ((m_mark->left_eye.y > (m_mark->face_box.y + m_mark->face_box.height))
		|| (m_mark->right_eye.y > (m_mark->face_box.y + m_mark->face_box.height)))
	{
		return 0;
	}

	if ((m_mark->mouth_left.y > (m_mark->face_box.y + m_mark->face_box.height))
		|| (m_mark->mouth_right.y > (m_mark->face_box.y + m_mark->face_box.height)))
	{
		return 0;
	}

	if ((m_mark->face_box.height < 80)
		|| (m_mark->face_box.width < 80))
	{
		return 0;
	}

    int Sface = m_mark->face_box.width * m_mark->face_box.height;
    int Simg = m_img.cols * m_img.rows;

    m_size = (float) Sface / Simg;

    m_scores["size_score"] = m_size;

    return m_size;
}

float Quality::position_score(void)
{
	float positi_s=0;

	int height = m_img.rows;
	int width = m_img.cols;

	int picture_x_positon = width / 2;
	int picture_y_positon = height / 2;

	int face_x_position = m_mark->face_box.x + m_mark->face_box.width / 2;
	int face_y_position = m_mark->face_box.y + m_mark->face_box.height / 2;

	m_pos = sqrt((picture_x_positon-=face_x_position)*picture_x_positon+(picture_y_positon-=face_y_position)*picture_y_positon);
	m_pos /= sqrt(width * width + height * height);
	m_pos = 1.0 - m_pos;

	m_scores["position_score"] = m_pos;

	return m_pos;
}

float Quality::angle_score(void)
{
	int delta_y = abs(m_mark->right_eye.y - m_mark->left_eye.y);
	int delta_x = abs(m_mark->right_eye.x - m_mark->left_eye.x);
	float a = atan(float(delta_y) / float(delta_x));
	m_angle = 1 - (fabs(a) * 2 / PI);

    m_scores["angle_score"] = m_angle;

    return m_angle;
}

float Quality::contrast_score(void)
{
    int i = 0;
    int j = 0;
    int hist[256] = {0};

    Mat gray;

	if (m_img.channels() == 3)
    {
		cvtColor(m_img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = m_img;
    }

	int width_bound = m_mark->face_box.x + m_mark->face_box.width;
	int height_bound = m_mark->face_box.y + m_mark->face_box.height;

	for (i = m_mark->face_box.x; i < height_bound; i++)
	{
		for (j = m_mark->face_box.y; j < width_bound; j++)
		{
			int idx = gray.at<uchar>(i,j);
			hist[idx]++;
		}
	}

    float square_sum = 0.0;
    for (i = 0; i < 256; i++)
    {
        float delta = (float)hist[i] / (m_mark->face_box.width * m_mark->face_box.height) - 1 / 256;
        delta *= delta;
        square_sum += delta;
    }

    m_contrast = 1.0 - sqrt(square_sum);
    //cout << "contrast_score:"<<m_contrast <<endl;

    m_scores["contrast_score"] = m_contrast;

    return m_contrast;
}

float Quality::brightness_score(void)
{
	Mat gray;

	if (m_img.channels() == 3)
    {
		cvtColor(m_img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = m_img;
    }

	//Mat eq_img;

    //equalizeHist(gray, eq_img);

	//Mat face_roi = gray(m_mark->face_box);

    float mean_gray = mean(gray)[0];
    //float mean_eq = mean(eq_img)[0];
    m_brightness = 1.0 - fabs(mean_gray - 128) / 255;
    //cout << "bright_score:"<<m_brightness<<endl;
	m_scores["brightness_score"] = m_brightness;

	return m_brightness;
}

float Quality::clarity_score(void)
{
    Mat gray;

    if (m_img.channels() == 3)
    {
       cvtColor(m_img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = m_img;
    }

    Mat face_roi = gray(m_mark->face_box);

    Mat sobel_gray;
    Mat sobel_blur;
    Mat blur_img;

    GaussianBlur(face_roi, blur_img, Size(13, 13), 10, 10);

    Sobel(face_roi, sobel_gray, -1, 1, 1);
    Sobel(blur_img, sobel_blur, -1, 1, 1);

    float e_g = mean(sobel_gray)[0];
    float e_b = mean(sobel_blur)[0];

    m_clarity = fabs(e_g - e_b)/e_g;
    //cout<<"clarity_score:"<<m_clarity<<endl;

    m_scores["clarity_score"] = m_clarity;

    return m_clarity;
}

static cv::Vec3d RotationMatrix2Euler(const cv::Matx33d& rotation_matrix)
{
    double q0 = sqrt(1 + rotation_matrix(0, 0) + rotation_matrix(1, 1) + rotation_matrix(2, 2)) / 2.0;
    double q1 = (rotation_matrix(2, 1) - rotation_matrix(1, 2)) / (4.0*q0);
    double q2 = (rotation_matrix(0, 2) - rotation_matrix(2, 0)) / (4.0*q0);
    double q3 = (rotation_matrix(1, 0) - rotation_matrix(0, 1)) / (4.0*q0);

    double t1 = 2.0 * (q0*q2 + q1*q3);

    double yaw = asin(2.0 * (q0*q2 + q1*q3));
    double pitch = atan2(2.0 * (q0*q1 - q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3);
    double roll = atan2(2.0 * (q0*q3 - q1*q2), q0*q0 + q1*q1 - q2*q2 - q3*q3);

	yaw = fabs(yaw) / CV_PI * 180;
	pitch = fabs(pitch) / CV_PI * 180;
	roll = fabs(roll) / CV_PI * 180;
	//printf("yaw=%.2lf,pitch=%.2lf,roll=%.2lf\n", yaw, pitch, roll);

	yaw = (yaw >= 90) ? 180 - yaw : yaw;
	pitch = (pitch >= 90) ? 180 - pitch : pitch;
	roll = (roll >= 90) ? 180 - roll : roll;

	//printf("yaw=%.2lf,pitch=%.2lf,roll=%.2lf\n", yaw, pitch, roll);

    return cv::Vec3d(yaw, pitch, roll);
}

cv::Vec3d Quality::pose_score(void)
{
    // 2D image points. If you change the image, you need to change vector
    std::vector<cv::Point2d> image_points;

    image_points.push_back( cv::Point2d(m_mark->nose.x, m_mark->nose.y));    // Nose tip
    image_points.push_back( cv::Point2d(m_mark->chin.x, m_mark->chin.y));    // Chin
    image_points.push_back( cv::Point2d(m_mark->left_eye_outer.x, m_mark->left_eye_outer.y));     // Left eye left corner
    image_points.push_back( cv::Point2d(m_mark->right_eye_outer.x, m_mark->right_eye_outer.y));    // Right eye right corner
    image_points.push_back( cv::Point2d(m_mark->mouth_left.x, m_mark->mouth_left.y));    // Left Mouth corner
    image_points.push_back( cv::Point2d(m_mark->mouth_right.x, m_mark->mouth_right.y));    // Right mouth corner

    // 3D model points.
    std::vector<cv::Point3d> model_points;
    model_points.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));               // Nose tip
    model_points.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));          // Chin
    model_points.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // Left eye left corner
    model_points.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // Right eye right corner
    model_points.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));      // Left Mouth corner
    model_points.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));       // Right mouth corner

    // Camera internals
    double focal_length = m_img.cols; // Approximate focal length.
    Point2d center = cv::Point2d(m_img.cols/2, m_img.rows/2);
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion

    //cout << "Camera Matrix " << endl << camera_matrix << endl ;
    // Output rotation and translation
    cv::Mat rotation_vector; // Rotation in axis-angle form
    cv::Mat translation_vector;

    // Solve for pose
    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);


    // Project a 3D point (0, 0, 1000.0) onto the image plane.
    // We use this to draw a line sticking out of the nose

    vector<Point3d> nose_end_point3D;
    vector<Point2d> nose_end_point2D;
    nose_end_point3D.push_back(Point3d(0,0,1000.0));

    cv::projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);

	#if 0
    for(int i=0; i < image_points.size(); i++)
    {
        circle(m_img, image_points[i], 3, Scalar(0,0,255), -1);
    }
    cv::line(m_img, image_points[0], nose_end_point2D[0], cv::Scalar(255,0,0), 2);

    cout << "Rotation Vector " << endl << rotation_vector << endl;
    cout << "Translation Vector" << endl << translation_vector << endl;
    cout <<  nose_end_point2D << endl;
	#endif

    Matx33d rotation_matrix;
    Rodrigues(rotation_vector, rotation_matrix);
    m_pose_param = RotationMatrix2Euler(rotation_matrix);

	m_scores["yaw_score"] = m_pose_param[0];
	m_scores["pitch_score"] = m_pose_param[1];
	m_scores["roll_score"] = m_pose_param[2];

	return m_pose_param;
}

float Score(float param[], int len)
{
    assert(len==8);

    PyObject *pName,*pModule,*pDict,*pFunc,*pArgs;
    // 载入名为CalcScore的脚本
    //pName = PyString_FromString("CalcScore");
    pName = PyUnicode_FromString("CalcScore");
    pModule = PyImport_Import(pName);
    if ( !pModule ) {
        printf("can't find CalcScore.py");
        //getchar();
        return -1;
    }

    pDict = PyModule_GetDict(pModule);
    if ( !pDict ) {
        return -1;
    }

    // 找出函数名为CalcScore的函数
    printf("----------------------\n");
    pFunc = PyDict_GetItemString(pDict, "CalcScore");
    if ( !pFunc || !PyCallable_Check(pFunc) ) {
        printf("can't find function [CalcScore]");
        //getchar();
        return -1;
     }

    // 参数进栈
    *pArgs;
    pArgs = PyTuple_New(len);
    //  PyObject* Py_BuildValue(char *format, ...)
    //  把C++的变量转换成一个Python对象。当需要从
    //  C++传递变量到Python时，就会使用这个函数。此函数
    //  有点类似C的printf，但格式不同。常用的格式有
    //  s 表示字符串，
    //  i 表示整型变量，
    //  f 表示浮点数，
    //  O 表示一个Python对象。
    for (int i=0; i<len; ++i) {
        PyTuple_SetItem(pArgs, i, Py_BuildValue("f", param[i]));
    }

    // 调用Python函数
    PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
    char* result = PyUnicode_AsUTF8(pResult);
    printf("Result of call: %s\n", result);
    float ret = atof(result);

    Py_DECREF(pResult);
    Py_DECREF(pName);
    Py_DECREF(pArgs);
    Py_DECREF(pModule);

    return ret;
}


float Quality::quality_score(void)
{

    float param[8] = {m_scores["size_score"], m_scores["position_score"], m_scores["yaw_score"], m_scores["pitch_score"],
		m_scores["roll_score"],m_scores["contrast_score"],m_scores["clarity_score"],m_scores["brightness_score"]};
     m_total = Score(param, 8);
    m_scores["quality_score"] = m_total;
    return m_total;
}


bool Quality::is_pass(void)
{
	if (m_scores["angle_score"] < ANGLE_THRESHOLD
		|| m_scores["position_score"] < POS_THRESHOLD
		|| m_scores["size_score"] < SIZE_THRESHOLD
		|| m_scores["brightness_score"] < BRIGHT_THRESHOLD
		|| m_scores["contrast_score"] < CONTRAST_THRESHOLD
		|| m_scores["clarity_score"] < CLARITY_THRESHOLD
		|| m_scores["yaw_score"] >= YAW_THRESHOLD
		|| m_scores["pitch_score"] >= PITCH_THRESHOLD
		|| m_scores["roll_score"] >= ROLL_THRESHOLD
		|| m_scores["emotion"] < EMOTION_THRESHOLD
		|| m_scores["occlusion"] < OCCLUSION_THRESHOLD)
	{
		return false;
	}

	return true;
}

void Quality::show_all_scores(void)
{
    std::map<string, float>::iterator iter;
    for (iter = m_scores.begin(); iter != m_scores.end(); ++iter)
    {
        cout << iter->first << ": " << iter->second << '\n';
    }
}

void Quality::cal_all_scores(Mat img, landmark_t *mark)
{
	m_mark = mark;
	m_img = img;

    size_score();
    position_score();
    angle_score();
    contrast_score();
    brightness_score();
    clarity_score();
    pose_score();
    quality_score();
}

float Quality::at(const std::string &name)
{
    return m_scores.at(name);
}

void Quality::set(const std::string &name, float score)
{
	m_scores[name] = score;
}

void Quality::add(const std::string &name, float score)
{
    m_scores.insert(std::pair<string, float>(name, score));
}
