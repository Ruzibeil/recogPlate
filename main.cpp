#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#include "DetectRegion.h"
#include "OCR.h"
using namespace cv;

bool verifySizes(RotatedRect candiates);
Mat histseq( Mat in);

int main(int argc,char **argv)
{


	
	//svm to remove valid car plates
	FileStorage fs;
	fs.open("SVM.xml",FileStorage::READ);
	Mat SVM_TrainingData;
	Mat SVM_Classes;
	fs["TrainingData"]>>SVM_TrainingData;//!>写入到svm_TrainingData
	fs["classes"]>>SVM_Classes;

	//!>set svm params
	CvSVMParams SVM_params;
	SVM_params.svm_type = CvSVM::C_SVC;
	SVM_params.kernel_type = CvSVM::LINEAR;
	SVM_params.degree = 0;
	SVM_params.gamma =1;
	SVM_params.coef0 = 0;
	SVM_params.C =1;
	SVM_params.nu = 0;
	SVM_params.p = 0;
	SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER,100,0.01);


	//train
	CvSVM svmclassifier(SVM_TrainingData,SVM_Classes,Mat(),Mat(),SVM_params);

	//!> detect regions
	string fileName;
	cin>>fileName;
	
	Mat input_image = imread(fileName);
// 	imshow("img",input_image);
//	waitKey();
	DetectRegion dect;
	//dect.setFileName(fileName);
	dect.saveRegions = true;
	vector<Plate> possible_regions = dect.run(input_image);
	
	cout<<"wait..."<<endl;
// 
// 	// use svm model to pick up possible_regions
// 
	vector<Plate> plates;//!>SVM classifey plates
	for (int i = 0;i<possible_regions.size();i++)
	{
		Mat img = possible_regions[i].m_mPlateImage;
		Mat p = img.reshape(1,1);//!>转换成1行m特征,第一个参数为通道数
		p.convertTo(p,CV_32FC1);
		int response = (int)svmclassifier.predict(p);
		if(response==1)//!>设1为正样本
		{
			imshow("img1",img);
			waitKey();
			plates.push_back(possible_regions[i]);
		}
	}
	

	//plate recognition
	OCR ocr("OCR.xml");
	for (int plate_index = 0;plate_index<plates.size();plate_index++)
	{
		Plate plate = plates[plate_index];
		ocr.run(&plate);//!> 每一个牌子的字符集合在plate
		string plateLicense = plate.str();
		cout<<"==================="<<endl;
		cout<<"License:"<<plateLicense<<endl;
		cout<<"==================="<<endl;

	}




	

	return 0;
}

