#include "OCR.h"

const int OCR::numCharacters  = 30;
const char OCR:: strCharacters[] = {'0','1','2','3','4','5','6','7','8','9','B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z'}; 
//!> train ANN
OCR::OCR(string OCRNAME)
{
	charSize = 20;
	FileStorage fs;
	fs.open(OCRNAME,FileStorage::READ);
	Mat TrainData;
	Mat ClassData;
	fs["TrainingDataF15"]>>TrainData;
	fs["classes"]>>ClassData; //!> 671*1,被归一化到一列
	
	train(TrainData,ClassData,10);//!>隐藏层的节点数量为10
}
void OCR:: train(Mat TrainData,Mat ClassData,int nlayers)
{
	Mat layers(1,3,CV_32SC1);//!> 只有一个隐藏层
	layers.at<int>(0) = TrainData.cols;
	layers.at<int>(1) = nlayers;
	layers.at<int>(2) = numCharacters;
	ann.create(layers,CvANN_MLP::SIGMOID_SYM,1,1);

	Mat trainClasses;
	trainClasses.create(TrainData.rows,numCharacters,CV_32FC1);

	// one hot encoder
	for (int i = 0;i<trainClasses.rows;i++)
	{
		for (int j = 0;j< numCharacters;j++)
		{
			//!>classdata的数据是按从0到29分布的
			if (ClassData.at<int>(i) == j)
				trainClasses.at<float>(i,j)=1.0f;
			else
				trainClasses.at<float>(i,j) = 0.0f;
		}
	}
	Mat weights(1,TrainData.rows,CV_32FC1,Scalar::all(1));
	//!>RPROP算法的基本原理为：首先为各权重变化赋一个初始值，设定权重变化加速因子与减速因子，
	//!>在网络前馈迭代中当连续误差梯度符号不变时，采用加速策略，加快训练速度；
	//!>当连续误差梯度符号变化时，采用减速策略，以期稳定收敛。
	//!>网络结合当前误差梯度符号与变化步长实现BP，
	//!>同时，为了避免网络学习发生振荡或下溢，算法要求设定权重变化的上下限。
	
	ann.train(TrainData,trainClasses,weights);
	trained = true;
}
//!> extract feature

bool OCR:: verifySizes(Mat r)
{
	//!>长宽比 0.2~1.35aspect
	float aspect = 45.0f/77.0f;
	float error = 0.35;
	float minaspect = 0.2;
	float maxaspect = aspect + error *aspect;
	float chars_aspect =static_cast<float>(r.cols)/static_cast<float>(r.rows);
	float minHeight = 15;
	float maxHeight = 28;

	//!>原svm黑色为字，阈值话后白色为字，判断这一区域的比例
	float area = countNonZero(r);
	float chars_area = r.cols*r.rows;
	float perWhite = area/chars_area;
	//per>0.8 认为svm后的是黑块
	if (perWhite<0.7&&r.rows>=minHeight&&r.rows<=maxHeight
		&&chars_aspect>minaspect&&chars_aspect<maxaspect)

		return true;
	return false;


}
Mat OCR:: preprocessChar(Mat in)
{
	Mat out;
	//!> 仿射变换，一般用一个2x3的矩阵表示
	int width = in.cols;
	int height = in.rows;
	Mat transformMatrix = Mat::eye(2,3,CV_32F);
	int m = width > height?width:height;
	//translate
	transformMatrix.at<float>(0,2) = m/2 - width/2;
	transformMatrix.at<float>(1,2) = m/2 - height/2;
	Mat warpImage(m,m,in.type());
	warpAffine(in,warpImage,transformMatrix,warpImage.size()
		,INTER_LINEAR,BORDER_CONSTANT,Scalar(0));//scalar(border value)
	resize(warpImage,warpImage,Size(charSize,charSize));
	return warpImage;
}
// Char segment
vector<CharSegment>OCR:: segment(Plate plate)
{
	// in and out
	Mat input = plate.m_mPlateImage;
	vector<CharSegment> output;
	

	// binary
	Mat img_threshold;
	threshold(input,img_threshold,60,255,CV_THRESH_BINARY_INV);
	
	// contours
	Mat img_contours;
	img_threshold.copyTo(img_contours);
	vector<vector<Point>> contours;
	findContours(img_contours,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
	
	//draw blue contours on a white image for debug
	Mat result;
	img_threshold.copyTo(result);
	cvtColor(result,result,CV_GRAY2BGR);
	drawContours(result,contours,-1,Scalar(255,0,0));
	// !>verifysize
	if (DEBUG)
		imshow("contours",result);
		

	vector<vector<Point>> ::iterator its = contours.begin();
	for (;its!=contours.end();its++)
	{
		Rect mr = boundingRect(Mat(*its));
		//crop image
		Mat auxRoi(img_threshold,mr);
		if (verifySizes(auxRoi))
		{
			auxRoi = preprocessChar(auxRoi);// 20x20
			output.push_back(CharSegment(auxRoi,mr));
			rectangle(result,mr,Scalar(255,255,255));
		}
	}
	if(DEBUG){
		imshow("auxROI",result);
		waitKey();
	}

	return output;
}


//Extract features
Mat OCR:: ProjectedHisgram(Mat img,int t)
{
	// by horizontal or vertical
	int sz = t? img.rows:img.cols;
	Mat mhist = Mat::zeros(1,sz,CV_32F);
	for (int i = 0;i<sz;i++)
	{
		Mat data = t?img.row(i):img.col(i);//!>行矩阵或者是列矩阵
		mhist.at<float>(i) = countNonZero(data);
	}
	//Norm histogram
	double min,max;
	minMaxLoc(mhist,&min,&max);
	if (max >0)
		mhist.convertTo(mhist,-1,1.0f/max,0);//!>-1表示和原数据类型一致
	return mhist;
}

Mat OCR::features(Mat img,int lowSize)
{
	Mat vhist = ProjectedHisgram(img ,VERTICAL);
	Mat hhist = ProjectedHisgram(img,HORIZONTAL);
	Mat lowData;
	resize(img,lowData,Size(lowSize,lowSize));
	
	int numcols = vhist.cols + hhist.cols +lowData.cols*lowData.rows;
	Mat features = Mat::zeros(1,numcols,CV_32F);
	int j = 0;
	for (int i = 0; i< vhist.cols;i++)
	{
		features.at<float>(j) = vhist.at<float>(i);
		j++;
	}
	for (int i = 0; i< hhist.cols;i++)
	{
		features.at<float>(j) = hhist.at<float>(i);
		j++;
	}
	for (int x = 0;x<lowData.rows;x++)
	{
		for (int y = 0; y<lowData.cols;y++)
		{
			features.at<float>(j)=(float)lowData.at<unsigned char>(x,y);
			j++;
		}
	}
	return features;
}

//!> test
int OCR::classify(Mat f)
{
	int result = -1;
	Mat output(1,numCharacters,CV_32FC1);
	ann.predict(f,output);
	Point maxLoc;
	double maxVal;
	minMaxLoc(output,0,&maxVal,0,&maxLoc);
	return maxLoc.x;

}

//!> run
string OCR::run(Plate *input)
{
	vector<CharSegment> CharSets = segment(*input);
	for (int char_index = 0;char_index<CharSets.size();char_index++)
	{
		Mat ch = preprocessChar((CharSets[char_index].img));
		if(DEBUG)
		{
			char filename[30];
			sprintf_s(filename,"Chars_%d",char_index);
			imwrite(filename,ch);
		}
		Mat feature = features(ch,15);
		int character = classify(feature);//!>判断的是第几个，即字符数组的下标
		input->chars.push_back(strCharacters[character]);
		input->charsPos.push_back(CharSets[char_index].pos);
	}
	return "-";
}
OCR::~OCR(void)
{
}

