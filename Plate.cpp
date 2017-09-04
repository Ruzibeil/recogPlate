#include "Plate.h"

//  charpos 收集了一个车牌的字符的矩形
string Plate::str(){
	string result = "";
	//!>oreder array
	vector <int> orderIndex;
	vector <int> xposition;
	// init charpos into order array
	for (int i = 0;i <charsPos.size();i++)
	{
		orderIndex.push_back(i);
		xposition.push_back(charsPos[i].x);
	}
	//!>升序排列，每一轮选择最小得数置于最前面

	for (unsigned int i = 0;i<charsPos.size();i++)
	{
		int minIdx = i;
		float minXpos = xposition[i];
		for (unsigned int j=i;j<charsPos.size();j++)
		{
			if(xposition[j]<minXpos){
				minIdx = j;
				minXpos = xposition[minIdx];
			}

		}
		//!>交换第i个位置 和第i个车牌字符
		int swap_Xorder = orderIndex[i];
		int swap_MinOrder = orderIndex[minIdx];
		orderIndex[i] = swap_MinOrder;
		orderIndex[minIdx] = swap_Xorder;
		
		//!> 需要结合orderIndex ，同步交换 车牌字符
		float swap_xpostion = xposition[i];
		float swap_xpostionMin = xposition[minIdx];
		xposition[i] = swap_xpostionMin;//!>使第i个为最小值
		xposition[minIdx] = swap_xpostion;
	}

	for(int i = 0;i<chars.size();i++)
		result = result + chars[orderIndex[i]];

	return result;
}



