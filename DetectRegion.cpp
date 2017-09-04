#include "DetectRegion.h"


void DetectRegion:: setFileName(string f)
{
	m_strFileName = f;

}

bool DetectRegion::verifySizes(RotatedRect candiates)
{
	float error = 0.4f;
	const float aspect=4.7272f;//!>���ƿ���Ը�(520/110)
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
		merge(hsvsplit,hsv);//!> �������ǰ��ע�����ز�������
		cvtColor(hsv,out,CV_HSV2BGR);

	}
	else if (in.channels() ==1)
	{
		equalizeHist(in,out);
	}
	//imshow("out",out);

	return out;
}

//!> rect-> floodfill->mask->pointinserts->verifysize->minrect-> ������ת�ķ���任->�õ���ת֮���ԭͼ->�ָ� ->ֱ��ͼ���⻯
vector<Plate> DetectRegion::segment(Mat input)
{
	vector<Plate> output;
	//Mat Input = imread("1.jpg");
	/*imshow("orign",Input);*/
	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5,5));    

	//!>Finde vertical lines. Car plates have high density of vertical lines ɾ��û����ֱ�߲���
	Mat img_sobel;
	Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);//!>ֻ��ˮƽ���� xorder 1 kerner size =3
	/*imshow("Sobel",img_sobel);*/

	//
	Mat img_threshold;
	threshold(img_sobel,img_threshold,0,255,CV_THRESH_OTSU+CV_THRESH_BINARY);
	/*imshow("Threshold",img_threshold);*/
	

	Mat element = getStructuringElement(MORPH_RECT,Size(17,3));
	morphologyEx(img_threshold,img_threshold,CV_MOP_CLOSE,element);
	/*imshow("Close",img_threshold);*/
	//cout<<"wait..."<<endl;
	vector<vector<Point>> contours;//!>����ı߽�
	vector <RotatedRect> rects;//!>���պ��ʵı߿�����
	findContours(img_threshold,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);

	vector<vector<Point>>::iterator it;
	for (it = contours.begin();it!=contours.end();)
	{
		RotatedRect mr = minAreaRect(Mat(*it));//!>���ط�ʽ
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
		//!>����mask���õ����е���Ч����
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
		//!> ���շ���ľ���
		RotatedRect minRect = minAreaRect(pointInsert);
		//!>ɸѡmask
		if (verifySizes(minRect))
		{
			//rotate rectangle 
			Point2f rect_points[4];
			//!>�õ����ε��ĸ�����
			minRect.points(rect_points);
			//for ( int j = 0;j<4;j++)
				/*line(result,rect_points[j],rect_points[j+1],Scalar(0,0,255),1,8);*/


			//!>����������ת
			float r = (float)minRect.size.width/(float) minRect.size.height;
			float angle = minRect.angle;//!>xˮƽ����ʱ��ת�����α߳��ĽǶ�
			if(r <1)
				angle = 90+angle;
			Mat rotmat = getRotationMatrix2D(minRect.center,angle,1);//!>˳ʱ����ת


			//!>create and rotate img
			Mat imgrotated;
			warpAffine(input,imgrotated,rotmat,input.size(),CV_INTER_CUBIC);//!>����任
			//!>��Ҫ��rotmat���ʹ��

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
			resize(img_crop,resultResized,resultResized.size(),0,0,INTER_CUBIC);//!>����cvsize(width,height)
			//!>ֱ��ͼ���⻯
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

