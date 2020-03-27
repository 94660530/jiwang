#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include<string>
#include<math.h>
using namespace std;
using namespace cv;
#define uint unsigned int
#define ushort unsigned short 
char* CRC_tran(long i)
{
	long generate = 25;
	char g_add[50];
	char s_end[13];
	_itoa(i, s_end, 2);//����CRC���ܵ���
	int len_g = 5;
	i = i << (len_g - 1);//���Ӷ�Ӧλ����0
	_itoa(i, g_add, 2);
	int len_s = strlen(g_add);//len_s��������0�ļ�����
	long temp;
	while (len_s >= 5)//���Ͻ������������õ�����
	{
		temp = generate << (len_s - len_g);
		i = i ^ temp;
		_itoa(i, g_add, 2);
		len_s = strlen(g_add);//g_add�ŵ�������
	}
	char YS[5];
	switch (len_s) {
	case 0:
		YS[2] = 48;
		YS[3] = 48;
		YS[0] = 48;
		YS[1] = 48;
		YS[4] = '\0';
		break;
	case 1:
		YS[2] = 48;
		YS[3] = g_add[0];
		YS[0] = 48;
		YS[1] = 48;
		YS[4] = '\0';
		break;
	case 2:
		YS[2] = g_add[0];
		YS[3] = g_add[1];
		YS[0] = 48;
		YS[1] = 48;
		YS[4] = '\0';
		break;
	case 3:
		YS[2] = g_add[1];
		YS[3] = g_add[2];
		YS[0] = 48;
		YS[1] = g_add[0];
		YS[4] = '\0';
		break;
	default:
		YS[0] = g_add[0];
		YS[1] = g_add[1];
		YS[2] = g_add[2];
		YS[3] = g_add[3];
		YS[4] = '\0';
		break;
	}
	len_s = strlen(s_end);
	if (len_s < 8)
	{
		for (int j = 8 - len_s; j > 0; j--)
		{
			for (int i = len_s; i > 0; i--)
				s_end[i] = s_end[i - 1];
			s_end[0] = 48;
			len_s++;
		}
	}
	s_end[8] = '\0';
	strcat(s_end, YS);
	return s_end;
}
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
	uint temp = abs(frequence);
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
	int temp = abs(frequence);
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

			sum += cifang(i) * pwm_temp[i];
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
void DataToImg(const char* Dat_name)
{
	srand(time(NULL));
	int flag = 100000;
	// ���ô���
	int i = 0, j, k = 1;//i���ڱ�ʾ�Ѿ���¼���ڼ����ַ���,k���ڱ�ʾ�ڼ��Ŷ�ά��
	ushort Store[588][8] = { 0 };
	char CRCStore[588][13];
	string information;
	FILE* fp = fopen(Dat_name, "rb");
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
	string info = information;

	int count = 0;
	while (flag > 0)//�����ж��ڵ�ǰ��ά��ͼ�Ƿ������һ����
	{
		for (int p = 588 * count; p < 588 * (1 + count); p++)
		{
			Stage(info[p], Store[p - count * 588]);
		}

		Mat img = Mat::zeros(Size(1000, 1000), CV_8UC3);
		img.setTo(Scalar(255, 255, 255));              // ������ĻΪ��ɫ

		rectangle(img, Rect(5, 5, 5, 5), Scalar(0, 0, 0), -1);

		rectangle(img, Rect(15, 15, 35, 35), Scalar(0, 0, 0), -1);
		rectangle(img, Rect(20, 20, 25, 25), Scalar(255, 255, 255), -1);
		rectangle(img, Rect(25, 25, 15, 15), Scalar(0, 0, 0), -1);

		rectangle(img, Rect(15, 950, 35, 35), Scalar(0, 0, 0), -1);
		rectangle(img, Rect(20, 955, 25, 25), Scalar(255, 255, 255), -1);
		rectangle(img, Rect(25, 960, 15, 15), Scalar(0, 0, 0), -1);

		rectangle(img, Rect(950, 15, 35, 35), Scalar(0, 0, 0), -1);
		rectangle(img, Rect(955, 20, 25, 25), Scalar(255, 255, 255), -1);
		rectangle(img, Rect(960, 25, 15, 15), Scalar(0, 0, 0), -1);

		rectangle(img, Rect(950, 950, 35, 35), Scalar(0, 0, 0), -1);
		rectangle(img, Rect(955, 955, 25, 25), Scalar(255, 255, 255), -1);
		rectangle(img, Rect(960, 960, 15, 15), Scalar(0, 0, 0), -1);

		int x = 80, y = 80;
		for (i = 0; i < 588; i++)
		{
			if ((char)info[i + 588 * count] > 0)
			{
				Rect r(x, y, 10, 10);
				rectangle(img, r, Scalar(0, 0, 0), -1);
				x += 10;
			}
			else
			{
				Rect r(x, y, 10, 10);
				rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}

			for (j = 6; j >= 0; j--)
			{
				Rect r(x, y, 10, 10);
				if (Store[i][j] == 0)
					rectangle(img, r, Scalar(0, 0, 0), -1);
				else if (Store[i][j] == 1)
					rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}
			long total = 0;  //ת����CRC
			char* crc;
			for (int j = 0; j < 8; j++)
				total += Store[i][j] * pow(2, j);//��Ϊ����crc������������long�������Ȱ����ݴ�����ĳ�long
			crc = CRC_tran(total);
			for (j = 0; j < 12; j++)
				CRCStore[i][j] = *(crc + j);
			CRCStore[i][12] = '\0';

			for (j = 8; j < 12; j++)
			{
				Rect r(x, y, 10, 10);
				if (CRCStore[i][j] == '0')
					rectangle(img, r, Scalar(0, 0, 0), -1);
				else if (CRCStore[i][j] == '1')
					rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}

			if ((i + 1) % 7 == 0)
			{
				cout << endl;
				x = 80; y += 10;
			}
		}
		/*for (int a = flag; a < 588; a++)
		{
			for (j = 0; j < 12; j++)
			{
				Rect r(x, y, 10, 10);
				rectangle(img, r, Scalar(255, 255, 255), -1);
				x += 10;
			}
			if ((i + 1) % 7 == 0)
			{
				cout << endl;
				x = 80; y += 10;
			}
		}*/

		//imshow("����", img);

		string Img_Name = "QCode\\" + to_string(k) + ".png";
		imwrite(Img_Name, img);
		//waitKey(0);
		flag = flag - 588; k++; count++;
		//if (k > 50) break;
	}
	return;
}

