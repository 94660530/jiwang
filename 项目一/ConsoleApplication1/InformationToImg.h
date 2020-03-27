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
//��2��N�η�  
int cifang(int n)
{
	int i = 0, sum = 1;
	for (i = n; i > 0; i--)
	{
		sum *= 2;
	}
	return sum;
}
//ʮ��������ת���ɶ������� 
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
	//����	
	if (frequence < 0)
	{
		//�Ƚ�����ת�������� 
		temp = -frequence;

		//����������temp�Ķ����Ʋ������pwm_temp��	
		transform_data_zhengshu(temp, pwm_temp);

		//���������temp�Ķ�����pwm_tempȡ����ת����ʮ������ sum 
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

		//��ת��������ʮ������sum��һ 
		sum += 1;
		//printf("sum=%d\n",sum);

		//�����ת�ɶ�����pwm_table
		return transform_data_zhengshu(sum, pwm_table);
	}

	//���� 
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
	// ���ô���
	int i = 0, j, k = 1;//i���ڱ�ʾ�Ѿ���¼���ڼ����ַ���,k���ڱ�ʾ�ڼ��Ŷ�ά��
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
	while (flag > 0)//�����ж��ڵ�ǰ��ά��ͼ�Ƿ������һ����
	{
		Mat img = Mat::zeros(Size(400, 400), CV_8UC3);
		img.setTo(Scalar(255, 255, 255));              // ������ĻΪ��ɫ
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

		//imshow("����", img);

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
	cout << "�������ļ�;����";
	cin >> name;
	const char *ch = name.c_str();
	DataToImg(ch);
	return 0;
}
*/