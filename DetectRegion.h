#pragma once

#include <opencv2/ml/ml.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include "Plate.h"
using namespace  std;
using namespace  cv;
class DetectRegion
{
public:
	DetectRegion(){saveRegions = false;};
	
	string m_strFileName;
	bool saveRegions;
	vector<Plate> run(Mat input);
	void setFileName(string f);
private:
	vector<Plate> segment(Mat input);
	bool verifySizes(RotatedRect mr);
	Mat histseq(Mat in);

};

