#include "DetectRegion.h"


void DetectRegion:: setFileName(string f)
{
	m_strFileName = f;

}

bool DetectRegion::verifySizes(RotatedRect candiates)
{
	float error = 0.4f;
	const float aspect=4.7272f;//!>车牌宽除以高(520/110)
	//set min area and max area
	int min = 15*aspect*15;
	int max = 125*aspect*125;
	float rmin = aspect - aspect*error;
	float rmax = aspect + aspect*error;

	int area = candiates.size.height*candiates.size.width;
	float r = (float)candiates.size.width/(float)candiates.size.height;
	if(r<1) r= 1/r;
	if((area<min||area>max)||(r<rmin||r>rmax))
		return false;
	else
		return true;
}

Mat DetectRegion::histseq( Mat in)
{
	Mat out(in.size(),in.type());
	if (in.channels()==3)
	{
		Mat hsv;
		vector <Mat> hsvsplit;
		cvtColor(in,hsv,CV_BGR2HSV);
		split(hsv,hsvsplit);
		equalizeHist(hsvsplit[2],hsvsplit[2]);
		merge(hsvsplit,hsv);//!> 均衡后在前，注意重载参数类型
		cvtColor(hsv,out,CV_HSV2BGR);

	}
	else if (in.channels() ==1)
	{
		equalizeHist(in,out);
	}
	//imshow("out",out);

	return out;
}

//!> rect-> floodfill->mask->pointinserts->verifysize->minrect-> 矩阵旋转的仿射变换->得到旋转之后的原图->分割 ->直方图均衡化
vector<Plate> DetectRegion::segment(Mat input)
{
	vector<Plate> output;
	//Mat Input = imread("1.jpg");
	/*imshow("orign",Input);*/
	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5,5));    

	//!>Finde vertical lines. Car plates have high density of vertical lines 删除没有竖直边部分
	Mat img_sobel;
	Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);//!>只求水平导数 xorder 1 kerner size =3
	/*imshow("Sobel",img_sobel);*/

	//
	Mat img_threshold;
	threshold(img_sobel,img_threshold,0,255,CV_THRESH_OTSU+CV_THRESH_BINARY);
	/*imshow("Threshold",img_threshold);*/
	

	Mat element = getStructuringElement(MORPH_RECT,Size(17,3));
	morphologyEx(img_threshold,img_threshold,CV_MOP_CLOSE,element);
	/*imshow("Close",img_threshold);*/
	//cout<<"wait..."<<endl;
	vector<vector<Point>> contours;//!>最初的边界
	vector <RotatedRect> rects;//!>最终合适的边框区域
	findContours(img_threshold,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	vector<vector<Point>>::iterator it;
	for (it = contours.begin();it!=contours.end();)
	{
		RotatedRect mr = minAreaRect(Mat(*it));//!>重载方式
		if(!verifySizes(mr))
			it = contours.erase(it);
		else
		{
			it++;
			rects.push_back(mr);
		}
	}
	//cout<<"wait..."<<endl;
// 	Mat result(input.size(),CV_8UC3,Scalar::all(0));
// 	drawContours(result,contours,-1,Scalar(255,0,0));
	/*imshow("contours",result);*/
	//!> Do Floodfill



	
	for (int rect_index = 0;rect_index<rects.size();rect_index++)
	{
		/*circle(result,rects[rect_index].center,3,Scalar(0,255,0),-1);*/
		float  minsize = rects[rect_index].size.height>rects[rect_index].size.width?rects[rect_index].size.width:rects[rect_index].size.height;
		minsize = minsize - 0.5*minsize;
		srand( time(NULL));
		
		Mat mask(input.rows+2,input.cols+2,CV_8U,Scalar::all(0));
		int lowDiff = 30;
		int upDiff = 30;
		int connectivity = 4;
		int newMaskval = 255;
		int flags = connectivity +(newMaskval<<8)+FLOODFILL_FIXED_RANGE+FLOODFILL_MASK_ONLY;//!>32bit
		Rect ccomp;
		int NumSeeds = 10;
		//char maskname[20];
		for (int j=0;j<NumSeeds;j++)
		{
			Point seed;
			seed.x = (int)rects[rect_index].center.x +rand()%(int)minsize-minsize/2;
			seed.y = (int)rects[rect_index].center.y + rand()% (int)minsize - minsize/2;
			/*circle(result,seed,1,Scalar(0,255,255),-1);*/
			/*    imshow("circle",result)*/;
			int area = floodFill(input,mask,seed,Scalar(255,0,0),&ccomp,Scalar(lowDiff,lowDiff,lowDiff),Scalar(upDiff,upDiff,upDiff),flags);

		}
		// !>save mask
//  		stringstream maskss;
//  		maskss<<"mask_"<<rect_index<<".jpg";
// 		cout<<mask.rows<<mask.cols<<endl;
		
 		//imwrite(maskss.str(),mask);
// 		imshow("mask",mask);    
// 		waitKey();    
//		cout<<"wait..."<<endl;
		//!>处理mask，得到其中的有效部分
		vector<Point> pointInsert;
		Mat_ <uchar> ::iterator itmask =mask.begin<uchar>();
		Mat_ <uchar>::iterator end = mask.end<uchar>();
		for (;itmask!=end;itmask++)
		{
			if (*itmask ==255)
			{
				pointInsert.push_back(itmask.pos());

			}
		}
		//!> 最终放入的矩形
		RotatedRect minRect = minAreaRect(pointInsert);
		//!>筛选mask
		if (verifySizes(minRect))
		{
			//rotate rectangle 
			Point2f rect_points[4];
			//!>得到矩形的四个顶点
			minRect.points(rect_points);
			//for ( int j = 0;j<4;j++)
				/*line(result,rect_points[j],rect_points[j+1],Scalar(0,0,255),1,8);*/


			//!>处理矩阵的旋转
			float r = (float)minRect.size.width/(float) minRect.size.height;
			float angle = minRect.angle;//!>x水平轴逆时旋转至矩形边长的角度
			if(r <1)
				angle = 90+angle;
			Mat rotmat = getRotationMatrix2D(minRect.center,angle,1);//!>顺时针旋转


			//!>create and rotate img
			Mat imgrotated;
			warpAffine(input,imgrotated,rotmat,input.size(),CV_INTER_CUBIC);//!>仿射变换
			//!>需要和rotmat配合使用

			// crop image
			Size rect_size = minRect.size; //!>width and height 
			if (r<1)
			{
				std::swap(rect_size.width,rect_size.height);
			}
			Mat img_crop;
			getRectSubPix(imgrotated,rect_size,minRect.center,img_crop);

			Mat resultResized;
			resultResized.create(33,144,CV_8UC3);
			resize(img_crop,resultResized,resultResized.size(),0,0,INTER_CUBIC);//!>区分cvsize(width,height)
			//!>直方图均衡化
			Mat grayResult;
			grayResult.create(33,144,CV_8UC3);
			cvtColor(resultResized,grayResult,CV_BGR2GRAY);
			blur(grayResult,grayResult,Size(3,3));
			grayResult = histseq(grayResult);
			//imshow("img1",grayResult);
			//waitKey();
			char ss[50];
			if(saveRegions)
			{
			
			sprintf_s(ss,"chepai%d.bmp",rect_index);
			//ss <<"chepai"<<rect_index<<".bmp";
			//cout<<"save possible:"<<ss<<endl;
			//imwrite(ss,grayResult);
			//cout<<grayResult<<endl;
		
			}
			cout<<"wait"<<endl;
			output.push_back(Plate(grayResult,minRect.boundingRect()));


		}
	}
	//waitKey();
	return  output;
}

vector<Plate> DetectRegion::run(Mat input)
{
	vector<Plate> tmp = segment(input);
	return tmp;
}

