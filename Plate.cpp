#include "Plate.h"

//  charpos �ռ���һ�����Ƶ��ַ��ľ���
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
	//!>�������У�ÿһ��ѡ����С����������ǰ��

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
		//!>������i��λ�� �͵�i�������ַ�
		int swap_Xorder = orderIndex[i];
		int swap_MinOrder = orderIndex[minIdx];
		orderIndex[i] = swap_MinOrder;
		orderIndex[minIdx] = swap_Xorder;
		
		//!> ��Ҫ���orderIndex ��ͬ������ �����ַ�
		float swap_xpostion = xposition[i];
		float swap_xpostionMin = xposition[minIdx];
		xposition[i] = swap_xpostionMin;//!>ʹ��i��Ϊ��Сֵ
		xposition[minIdx] = swap_xpostion;
	}

	for(int i = 0;i<chars.size();i++)
		result = result + chars[orderIndex[i]];

	return result;
}



