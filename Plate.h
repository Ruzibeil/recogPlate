#ifndef PLATE_H
#define PLATE_H
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sstream>
#include <string>
using namespace cv;
using namespace std;
class Plate
{
public:
	Plate();
	
	Plate(Mat img, Rect pos):m_mPlateImage(img),m_rPosition(pos){}
	string str();
	Rect m_rPosition;
	Mat m_mPlateImage;
	vector<char> chars; //!>最后输出的车牌
	vector<Rect> charsPos;
};

#endif
