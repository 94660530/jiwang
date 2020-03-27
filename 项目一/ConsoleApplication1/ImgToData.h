#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>
#include <vector>

using namespace std;
using namespace cv;


/******************************************************************
	�ñ任�Ǽ�����Ƭ����һ��ƽ���Ҵ�ֱ������ͷ
	(����ά��û�з����ͶӰ ������ƽ�ƺ���ת)
*******************************************************************/
Mat transformCorner(Mat src, RotatedRect rect)
{
	Point center = rect.center;   //��ת����
	//circle(src, center, 2, Scalar(0, 0, 255), 2);
	//Size sz = Size(rect.size.width, rect.size.height);
	Point TopLeft = Point(cvRound(center.x), cvRound(center.y)) - Point(rect.size.height / 2, rect.size.width / 2);  //��ת���Ŀ��λ��
	TopLeft.x = TopLeft.x > src.cols ? src.cols : TopLeft.x;
	TopLeft.x = TopLeft.x < 0 ? 0 : TopLeft.x;
	TopLeft.y = TopLeft.y > src.rows ? src.rows : TopLeft.y;
	TopLeft.y = TopLeft.y < 0 ? 0 : TopLeft.y;

	//Point ButtonRight = (Point)center - Point(rect.size.width, rect.size.height);
	Rect RoiRect = Rect(TopLeft.x, TopLeft.y, rect.size.width, rect.size.height);   //��ͼ�ر�����
	double angle = rect.angle;        //��ת�Ƕ�
	Mat mask, roi, dst;                //dst�Ǳ���ת��ͼƬ roiΪ���ͼƬ maskΪ��ģ
	Mat image;						 //����ת���ͼƬ
	Size sz = src.size();             //��ת��ĳߴ�
	mask = Mat::zeros(src.size(), CV_8U);

	/************************************
	Ϊ��ģ��ɫ һ��Ϊ��ɫ
	��ΪRotatedRect ���͵ľ��β����׵�ȡ������ ����Ҫ���Ҳ�̫����
	����ҰѾ��ε��ĸ����㵱������ ����drawContours���
	************************************/
	vector<Point> contour;
	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++)
		contour.push_back(points[i]);
	vector<vector<Point>> contours;
	contours.push_back(contour);
	//drawContours(mask, contours, 0, Scalar(1), -1);

	/*��ͼ��Ȼ��Χ�����ľ���������ת*/
	src.copyTo(dst, mask);
	//roi = dst(RoiRect);
	Mat M = getRotationMatrix2D(center, angle, 1);
	warpAffine(dst, image, M, sz);
	roi = image(RoiRect);

	//imshow("image", image);
	//waitKey(0);

	return roi;
}


/********ͳ�����ص�*****/
double Rate(Mat& count)
{
	int number = 0;
	int allpixel = 0;
	for (int row = 0; row < count.rows; row++)
	{
		for (int col = 0; col < count.cols; col++)
		{
			if (count.at<uchar>(row, col) == 255)
			{
				number++;
			}
			allpixel++;
		}
	}
	//cout << (double)number / allpixel << endl;
	return (double)number / allpixel;
}

/***************�ж�����ͼ�����ײ������Ƿ�������*********************/
bool isCorner(Mat& image)
{
	/*******dstCopy�����Ƿ�ֹfindContours�޸�dstGray*******/
	/*******dstGray���滹��Ҫ��ͼ**************************/
	Mat mask, dstGopy;
	Mat dstGray;
	mask = image.clone();
	cvtColor(image, dstGray, COLOR_BGR2GRAY);
	threshold(dstGray, dstGray, 100, 255, THRESH_BINARY_INV);  //��ֵ�����������
	dstGopy = dstGray.clone();  //����
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(dstGopy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		//cout << i << endl;
		if (hierarchy[i][2] == -1 && hierarchy[i][3])
		{
			Rect rect = boundingRect(Mat(contours[i]));
			rectangle(image, rect, Scalar(0, 0, 255), 2);
			/******************��ͼ��֪������ľ��ο��ռ�ܿ��3/7***********************/
			if (rect.width < mask.cols * 2 / 7)      //2/7��Ϊ�˷�ֹһЩ΢С�ķ���
				continue;
			Mat result = dstGray(rect);
			if (Rate(result) > 0.75)       //0.75���Ҳ��Լ���ͼƬ�ľ���ֵ �ɸ����������(��������������)
			{
				rectangle(mask, rect, Scalar(0, 0, 255), 2);
				return true;
			}
		}
	}
	//imshow("dstGray", image);
	//imshow("mask", dstGray);
	return  false;
}

/*�ж�ͼ���Ƿ��ж�ά��*/
bool IsQr(Mat src)
{
	Mat src2;
	cvtColor(src, src2, COLOR_BGR2GRAY);
	blur(src2, src2, Size(3, 3));
	equalizeHist(src2, src2);

	/*��ֵ����ʵ����� ����ͼ�����Ҳ������� ����������*/
	threshold(src2, src2, 105, 255, THRESH_BINARY);

	/*imshow("test", src2);
	waitKey(0);*/

	vector<vector<Point>> contours, contours2;
	vector<Vec4i> hierarchy;
	findContours(src2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int ic = 0;
	int parentIdx = -1;

	/*������Ϊ�� �������ΪѰ�ҳ��������������ĸ�����*/
	/*ͦ���õ� ����ɸѡ������*/
	for (int i = 0; i < contours.size(); i++)
	{
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		else if (hierarchy[i][2] == -1)
		{
			parentIdx = -1;
			ic = 0;
		}
		if (ic >= 2)
		{
			if (hierarchy[i + 1][2] != i + 2)
				contours2.push_back(contours[parentIdx]);
			else
				contours2.push_back(contours[parentIdx + 1]);
			//drawContours(canvas, contours, parentIdx, Scalar(0, 0, 255), 1, 8);
			ic = 0;
			parentIdx = -1;
		}
	}

	vector<Point> center_all;  //center_all��ȡ��������
	for (int i = 0, count = 0; i < contours2.size(); i++)
	{
		//drawContours(canvas, contours, i, Scalar(0, 255, 0), 2);
		double area = contourArea(contours2[i]);
		if (area < 100)
			continue;
		/*���Ƹ߿��*/
		RotatedRect rect = minAreaRect(Mat(contours2[i]));
		double w = rect.size.width;
		double h = rect.size.height;
		double rate = min(w, h) / max(w, h);
		if (rate > 0.85)
		{
			return true;
		}
	}
	return false;
}

/*�ж��������겢����*/
Point m_leftup(Point2f a[4][4])
{
	Point res;
	res.x = INT16_MAX;
	res.y = INT16_MAX;
	int temp = INT16_MAX;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (a[i][j].x + a[i][j].y < temp) res.x = a[i][j].x, res.y = a[i][j].y, temp = a[i][j].x + a[i][j].y;
		}
	}
	return res;
}

/*�ж��������겢����*/
Point m_leftdown(Point2f a[4][4])
{
	Point res;
	res.x = -1;
	res.y = -1;
	int temp = INT16_MAX;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (a[i][j].x - a[i][j].y < temp) res.x = a[i][j].x, res.y = a[i][j].y, temp = a[i][j].x - a[i][j].y;
		}
	}
	return res;
}

/*�ж��������겢����*/
Point m_rightup(Point2f a[4][4])
{
	Point res;
	res.x = -1;
	res.y = -1;
	int temp = INT16_MAX;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (a[i][j].y - a[i][j].x < temp) res.x = a[i][j].x, res.y = a[i][j].y, temp = a[i][j].y - a[i][j].x;
		}
	}
	return res;
}

/*�ж��������겢����*/
Point m_rightdown(Point2f a[4][4])
{
	Point res;
	res.x = -1;
	res.y = -1;
	int temp = -1;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (a[i][j].y + a[i][j].x > temp) res.x = a[i][j].x, res.y = a[i][j].y, temp = a[i][j].y + a[i][j].x;
		}
	}
	return res;
}

bool CRCJudge(long i)//�жϴ������Ϣ�Ƿ�����
{
	long generate = 25;//11001
	char g_add[13];//�ű���������ʽ
	char s_end[13];
	_itoa(i, s_end, 2);//�������
	int len_s = strlen(s_end);
	_itoa(generate, g_add, 2);//CRC����ʽ
	int len_g = strlen(g_add);
	long temp;
	while (len_s >= len_g)//���Ͻ������������õ�����
	{
		temp = generate << (len_s - len_g);
		i = i ^ temp;
		_itoa(i, s_end, 2);
		len_s = strlen(s_end);
	}
	bool flag = true;
	for (int j = 0; j < len_s; j++)
		if (s_end[j] != '0')flag = false;
	return flag;
}

void ImageToData(const char* Path)
{
	String img_path = Path;
	vector<String> img;

	glob(img_path, img, false);

	size_t count = img.size();
	ofstream fout("out.bin", ios::out | ios::binary);
	ofstream fcrc("v1.bin", ios::out | ios::binary);

	for (size_t i = 1; i <= count; i++)
	{
		stringstream str;
		str << i << ".png";
		Mat image = imread(img_path + "\\" + str.str());

		/*cout << i << endl;

		if (i == 45)
		{
			system("pause");
		}*/

		long CRCtotal = 0;
		uchar temp = '\0';
		if (!image.empty())
		{
			if (image.size() == Size(1000, 1000))
			{
				cvtColor(image, image, COLOR_RGB2GRAY);
				for (int c = 0; c < 84; c++)//��
				{
					for (int t = 0; t < 7; t++)//��
					{
						bool symple = false;
						for (int r = 0; r < 12; r++)//λ
						{

							Scalar color = image.at<uchar>(85 + c * 10, 85 + r * 10 + t * 120);//(y,x)
							//cout << color[0];
							if (color[0] != 0)
							{
								switch (r)
								{
								case 0:
									symple = true;
									break;
								case 1:
									temp += 64;
									CRCtotal += 1024;
									break;
								case 2:
									temp += 32;
									CRCtotal += 512;
									break;
								case 3:
									temp += 16;
									CRCtotal += 256;
									break;
								case 4:
									temp += 8;
									CRCtotal += 128;
									break;
								case 5:
									temp += 4;
									CRCtotal += 64;
									break;
								case 6:
									temp += 2;
									CRCtotal += 32;
									break;
								case 7:
									temp += 1;
									CRCtotal += 16;
									break;
								case 8:
									CRCtotal += 8;
									break;
								case 9:
									CRCtotal += 4;
									break;
								case 10:
									CRCtotal += 2;
									break;
								case 11:
									CRCtotal += 1;
									break;
								default:
									break;
								}
							}
						}
						if ((CRCJudge(CRCtotal)) == false)
						{
							char b = 0;
							fcrc.write((char*)&b, sizeof(b));
						}
						else
						{
							char b = 255;
							fcrc.write((char*)&b, sizeof(b));
						}
						if (symple == true)temp = -temp;
						if (symple == true && temp == 0)temp = -128;
						if (symple == false && temp == 0)temp = 128;
						fout.write((char*)&temp, sizeof(temp));
						temp = '\0';
						CRCtotal = 0;
					}
				}
			}
			else
			{
				double start = (double)getTickCount();
				Mat backupMat = image.clone();
				Mat srcGray;            //canvasΪ���� ���ҵ��Ķ�λ����������
				Mat canvas = image.clone();
				Mat canvasGray;
				//pyrDown(src, src);  //ͼƬ����ʱʹ��
				//canvas = Mat::zeros(image.size(), CV_8UC3);

				/*�Ҷ��˲�ֱ��ͼ��ֵ�� ��߶Աȶ�*/

				cvtColor(image, srcGray, COLOR_BGR2GRAY);
				blur(srcGray, srcGray, Size(3, 3));
				equalizeHist(srcGray, srcGray);

				/*��ֵ����ʵ����� ����ͼ�����Ҳ������� ����������*/
				threshold(srcGray, srcGray, 110, 255, THRESH_BINARY);

				/*imshow("test",srcGray);
				waitKey(0);*/

				/*contours�ǵ�һ��Ѱ������*/
				/*contours2��ɸѡ��������*/
				vector<vector<Point>> contours, contours2;
				vector<Vec4i> hierarchy;
				findContours(srcGray, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
				int ic = 0;
				int parentIdx = -1;

				/*������Ϊ�� �������ΪѰ�ҳ��������������ĸ�����*/
				/*ͦ���õ� ����ɸѡ������*/
				for (int i = 0; i < contours.size(); i++)
				{
					if (hierarchy[i][2] != -1 && ic == 0)
					{
						parentIdx = i;
						ic++;
					}
					else if (hierarchy[i][2] != -1)
					{
						ic++;
					}
					else if (hierarchy[i][2] == -1)
					{
						parentIdx = -1;
						ic = 0;
					}
					if (ic >= 2)
					{
						if (hierarchy[i + 1][2] != i + 2)
							contours2.push_back(contours[parentIdx]);
						else
							contours2.push_back(contours[parentIdx + 1]);
						//drawContours(canvas, contours, parentIdx, Scalar(0, 0, 255), 1, 8);
						ic = 0;
						parentIdx = -1;
					}
				}

				vector<Point> center_all;  //center_all��ȡ��������
				Point2f points[4][4];//��¼ÿ��ͼ�Ķ�λ������
				for (int i = 0, count = 0; i < contours2.size(); i++)
				{
					/*drawContours(canvas, contours2, i, Scalar(0, 255, 0), 1);*/
					double area = contourArea(contours2[i]);
					if (area < 100)
						continue;
					/*���Ƹ߿��*/
					RotatedRect rect = minAreaRect(Mat(contours2[i]));
					double w = rect.size.width;
					double h = rect.size.height;
					double rate = min(w, h) / max(w, h);
					if (rate > 0.85)
					{
						Mat src = transformCorner(image, rect); //������ת���ͼƬ
						if (isCorner(src))
						{
							rect.points(points[count]);
							for (int i = 0; i < 4; i++)
							{
								if (points[count][i].x - (int)points[count][i].x > 0.5)
									points[count][i].x = (int)points[count][i].x + 1;
								if (points[count][i].y - (int)points[count][i].y > 0.5)
									points[count][i].y = (int)points[count][i].y + 1;
							}
							count++;
						}
						/*imshow("test", canvas);
						waitKey(0);*/
					}
				}

				//for(int i = 0;i < 4;i++)
				//	cout << points[0][i] << points[1][i] << points[2][i] << endl;

				center_all.push_back(m_leftup(points));
				center_all.push_back(m_leftdown(points));
				center_all.push_back(m_rightup(points));
				center_all.push_back(m_rightdown(points));



				
				//ʹ��Χ���������λ��
				//for (int i = 0; i < 4; i++)
				//{
				//	Scalar color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//	while (color[0] > 150)
				//	{
				//		switch (i)
				//		{
				//		case 0:
				//			color = srcGray.at<uchar>(center_all[i].y+15, center_all[i].x);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].y += 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;

				//			color = srcGray.at<uchar>(center_all[i].y, center_all[i].x+15);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].x += 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;
				//			center_all[i].y += 1;
				//			center_all[i].x += 1;
				//			break;
				//		case 1:
				//			color = srcGray.at<uchar>(center_all[i].y - 15, center_all[i].x);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].y -= 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;

				//			color = srcGray.at<uchar>(center_all[i].y, center_all[i].x + 15);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].x += 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;
				//			center_all[i].y -= 1;
				//			center_all[i].x += 1;
				//			break;
				//		case 2:
				//			color = srcGray.at<uchar>(center_all[i].y + 15, center_all[i].x);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].y += 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;

				//			color = srcGray.at<uchar>(center_all[i].y, center_all[i].x - 15);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].x -= 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;
				//			center_all[i].y += 1;
				//			center_all[i].x -= 1;
				//			break;
				//		case 3:
				//			color = srcGray.at<uchar>(center_all[i].y - 15, center_all[i].x);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].y -= 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;

				//			color = srcGray.at<uchar>(center_all[i].y, center_all[i].x - 15);
				//			if (color[0] < 150)
				//			{
				//				color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				while (color[0] > 150)
				//				{
				//					center_all[i].x -= 1;
				//					color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//				}
				//			}
				//			if (color[0] < 150)
				//				break;
				//			center_all[i].y -= 1;
				//			center_all[i].x -= 1;
				//			break;
				//		default:
				//			break;
				//		}
				//		color = srcGray.at<uchar>(center_all[i].y, center_all[i].x);
				//	}
				//	//cout << center_all[i] << endl;
				//}

				Point2f in[4];
				for (int i = 0; i < 4; i++)
				{
					in[i].x = center_all[i].x;
					in[i].y = center_all[i].y;
				}

				Point2f out[4] = { Point2f(0,0),Point2f(0,970) ,Point2f(970,0) ,Point2f(970,970) };

				Mat ma;

				//cout << center_all;

				

				warpPerspective(image, ma, getPerspectiveTransform(in, out), Size(970, 970));
				
				imshow("test", ma);
				waitKey(0);

				cvtColor(ma, ma, COLOR_RGB2GRAY);

				for (int c = 0; c < 84; c++)//��
				{
					for (int t = 0; t < 7; t++)//��
					{
						bool symple = false;
						for (int r = 0; r < 12; r++)//λ
						{
							Scalar color = ma.at<uchar>(70 + c * 10, 70 + r * 10 + t * 120);//(y,x)
							circle(ma, Point(70 + r * 10 + t * 120, 70 + c * 10), 1, Scalar(0, 0, 0));

							if (color[0] >= 150)
							{
								switch (r)
								{
								case 0:
									symple = true;
									break;
								case 1:
									temp += 64;
									CRCtotal += 1024;
									break;
								case 2:
									temp += 32;
									CRCtotal += 512;
									break;
								case 3:
									temp += 16;
									CRCtotal += 256;
									break;
								case 4:
									temp += 8;
									CRCtotal += 128;
									break;
								case 5:
									temp += 4;
									CRCtotal += 64;
									break;
								case 6:
									temp += 2;
									CRCtotal += 32;
									break;
								case 7:
									temp += 1;
									CRCtotal += 16;
									break;
								case 8:
									CRCtotal += 8;
									break;
								case 9:
									CRCtotal += 4;
									break;
								case 10:
									CRCtotal += 2;
									break;
								case 11:
									CRCtotal += 1;
									break;
								default:
									break;
								}
							}
						}
						if ((CRCJudge(CRCtotal)) == false)
						{
							char b = 0;
							fcrc.write((char*)&b, sizeof(b));
						}
						else
						{
							char b = 255;
							fcrc.write((char*)&b, sizeof(b));
						}
						if (symple == true)temp = -temp;
						if (symple == true && temp == 0)temp = -128;
						if (symple == false && temp == 0)temp = '\0';
						fout.write((char*)&temp, sizeof(temp));
						temp = '\0';
						CRCtotal = 0;
					}
				}
				imshow("test", ma);
				waitKey(0);

				double	time = ((double)getTickCount() - start) / getTickFrequency();
				cout << time;
			}
		}
	}
	fout.close();
	return;
}


