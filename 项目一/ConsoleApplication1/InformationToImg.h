#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include<string>
using namespace std;
using namespace cv;
#define uint unsigned int
#define ushort unsigned short
//求2的N次方  
int cifang(int n)
{
	int i = 0, sum = 1;
	for (i = n; i > 0; i--)
	{
		sum *= 2;
	}
	return sum;
}
//十进制正数转换成二进制数 
uint transform_data_zhengshu(char frequence, ushort* pwm_table)
{
	uint temp = frequence;
	int pwm_index = 0;

	while (temp)
	{
		pwm_table[pwm_index] = (temp & 0x01);
		temp = temp >> 1;
		pwm_index++;
	}
	return pwm_index - 1;
}

uint transform_data(char frequence, ushort* pwm_table)
{
	int temp = frequence;
	int pwm_index = 0;
	ushort pwm_temp[15] = { 0 };
	//负数	
	if (frequence < 0)
	{
		//先将负数转换成正数 
		temp = -frequence;

		//求出这个正数temp的二进制并存放在pwm_temp中	
		transform_data_zhengshu(temp, pwm_temp);

		//将这个正数temp的二进制pwm_temp取反并转换成十进制数 sum 
		int sum = 0;
		int i = 0;
		for (i = 0; i < 15; i++)
		{
			if (pwm_temp[i] == 0)
				pwm_temp[i] = 1;
			else
				pwm_temp[i] = 0;

			sum += cifang(i)*pwm_temp[i];
			pwm_temp[i] = 0;
		}

		//将转换出来的十进制数sum加一 
		sum += 1;
		//printf("sum=%d\n",sum);

		//最后再转成二进制pwm_table
		return transform_data_zhengshu(sum, pwm_table);
	}

	//正数 
	return transform_data_zhengshu(frequence, pwm_table);
}
void Stage(char frequence, ushort Ch[])
{

	uint off_set;

	off_set = transform_data(frequence, Ch);

	int i = 0;
	for (i = 7; i > off_set; i--)
	{
		Ch[i] = 0;
	}
}
void DataToImg(const char*Dat_name)
{
	int flag = 100000;
	// 设置窗口
	int i = 0, j, k = 1;//i用于表示已经记录到第几个字符了,k用于表示第几张二维码
	ushort Store[1280][8] = { 0 };
	string information;
	FILE *fp = fopen(Dat_name, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		information.resize(len);
		fread((void*)information.data(), 1, len, fp);
		fclose(fp);
	}
	else
	{
		printf("fopen error\n");
	}
	
	
	flag = information.size();

	for (int p = 0; p < information.size(); p++)
	{
		Stage(information[p], Store[p]);
	}
	while (flag > 0)//用于判定在当前二维码图是否是最后一张了
	{
		Mat img = Mat::zeros(Size(400, 400), CV_8UC3);
		img.setTo(Scalar(255, 255, 255));              // 设置屏幕为淡色
		circle(img, Point(20, 20), 20, Scalar(0, 0, 0));
		circle(img, Point(380, 20), 20, Scalar(0, 0, 0));
		circle(img, Point(20, 380), 20, Scalar(0, 0, 0));
		int x = 40, y = 40; int count = 1;
		for (; i < information.size() && count <= 128; i++, count++)
		{


			for (j = 7; j >= 0; j--)
			{
				Rect r(x, y, 10, 10);
				if (Store[i][j] == 0)
					rectangle(img, r, Scalar(0, 0, 0), -1);
				if (Store[i][j] == 1)
					rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}

			if ((i + 1) % 4 == 0)
			{
				cout << endl;
				x = 40; y += 10;
			}
		}
		for (int a = flag; a < 128; a++)
		{

			for (j = 0; j < 8; j++)
			{
				Rect r(x, y, 10, 10);
				rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}
			if ((i + 1) % 4 == 0)
			{
				cout << endl;
				x = 40; y += 10;
			}
		}

		//imshow("画板", img);

		string Img_Name = "QCode\\" + to_string(k) + ".png";
		imwrite(Img_Name, img);
		//waitKey(0);
		flag = flag - 128; k++;
	}
	return ;
}


/*
int main()
{
	string name;
	cout << "请输入文件途径：";
	cin >> name;
	const char *ch = name.c_str();
	DataToImg(ch);
	return 0;
}
*/