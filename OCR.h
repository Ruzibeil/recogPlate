#pragma once
#include "Plate.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
using namespace cv;
using namespace  std;
#define DEBUG  false
#define  HORIZONTAL 1
#define  VERTICAL 0
class CharSegment
{
public:
	CharSegment(){}
	CharSegment(Mat i,Rect p):img(i),pos(p){}
	Mat img;
	Rect pos;
};

class OCR
{
public:
	OCR(string OCRNAME );
	~OCR(void);
	void train(Mat TrainData,Mat ClassData,int nlayers);
	string run(Plate *input);
private:
	static const int numCharacters;
	static  const char strCharacters[];
	CvANN_MLP ann;
	bool trained;
	int charSize;// NormSize
	bool verifySizes(Mat r);
	int classify(Mat f);
	vector<CharSegment> segment(Plate plate);
	Mat preprocessChar(Mat in);
	Mat ProjectedHisgram(Mat img,int t);
	Mat features(Mat img,int lowSize);
};



