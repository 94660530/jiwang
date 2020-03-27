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
	该变换是假设照片是在一个平面且垂直与摄像头
	(即二维码没有仿射或投影 仅仅是平移和旋转)
*******************************************************************/
Mat transformCorner(Mat src, RotatedRect rect)
{
	Point center = rect.center;   //旋转中心
	//circle(src, center, 2, Scalar(0, 0, 255), 2);
	//Size sz = Size(rect.size.width, rect.size.height);
	Point TopLeft = Point(cvRound(center.x), cvRound(center.y)) - Point(rect.size.height / 2, rect.size.width / 2);  //旋转后的目标位置
	TopLeft.x = TopLeft.x > src.cols ? src.cols : TopLeft.x;
	TopLeft.x = TopLeft.x < 0 ? 0 : TopLeft.x;
	TopLeft.y = TopLeft.y > src.rows ? src.rows : TopLeft.y;
	TopLeft.y = TopLeft.y < 0 ? 0 : TopLeft.y;

	//Point ButtonRight = (Point)center - Point(rect.size.width, rect.size.height);
	Rect RoiRect = Rect(TopLeft.x, TopLeft.y, rect.size.width, rect.size.height);   //抠图必备矩形
	double angle = rect.angle;        //旋转角度
	Mat mask, roi, dst;                //dst是被旋转的图片 roi为输出图片 mask为掩模
	Mat image;						 //被旋转后的图片
	Size sz = src.size();             //旋转后的尺寸
	mask = Mat::zeros(src.size(), CV_8U);

	/************************************
	为掩模上色 一般为白色
	因为RotatedRect 类型的矩形不容易调取内像素 （主要是我不太懂）
	因此我把矩形的四个顶点当成轮廓 再用drawContours填充
	************************************/
	vector<Point> contour;
	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++)
		contour.push_back(points[i]);
	vector<vector<Point>> contours;
	contours.push_back(contour);
	//drawContours(mask, contours, 0, Scalar(1), -1);

	/*抠图，然后围绕中心矩阵中心旋转*/
	src.copyTo(dst, mask);
	//roi = dst(RoiRect);
	Mat M = getRotationMatrix2D(center, angle, 1);
	warpAffine(dst, image, M, sz);
	roi = image(RoiRect);

	//imshow("image", image);
	//waitKey(0);

	return roi;
}


/********统计像素点*****/
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

/***************判断输入图像的最底层轮廓是否有特征*********************/
bool isCorner(Mat& image)
{
	/*******dstCopy作用是防止findContours修改dstGray*******/
	/*******dstGray后面还需要抠图**************************/
	Mat mask, dstGopy;
	Mat dstGray;
	mask = image.clone();
	cvtColor(image, dstGray, COLOR_BGR2GRAY);
	threshold(dstGray, dstGray, 100, 255, THRESH_BINARY_INV);  //阈值根据情况而定
	dstGopy = dstGray.clone();  //备份
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
			/******************由图可知最里面的矩形宽度占总宽的3/7***********************/
			if (rect.width < mask.cols * 2 / 7)      //2/7是为了防止一些微小的仿射
				continue;
			Mat result = dstGray(rect);
			if (Rate(result) > 0.75)       //0.75是我测试几张图片的经验值 可根据情况设置(测试数量并不多)
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

/*判断图中是否有二维码*/
bool IsQr(Mat src)
{
	Mat src2;
	cvtColor(src, src2, COLOR_BGR2GRAY);
	blur(src2, src2, Size(3, 3));
	equalizeHist(src2, src2);

	/*阈值根据实际情况 如视图中已找不到特征 可适量调整*/
	threshold(src2, src2, 105, 255, THRESH_BINARY);

	/*imshow("test", src2);
	waitKey(0);*/

	vector<vector<Point>> contours, contours2;
	vector<Vec4i> hierarchy;
	findContours(src2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int ic = 0;
	int parentIdx = -1;

	/*个人认为： 下面程序为寻找出有两个子轮廓的父轮廓*/
	/*挺好用的 几乎筛选出来了*/
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

	vector<Point> center_all;  //center_all获取特性中心
	for (int i = 0, count = 0; i < contours2.size(); i++)
	{
		//drawContours(canvas, contours, i, Scalar(0, 255, 0), 2);
		double area = contourArea(contours2[i]);
		if (area < 100)
			continue;
		/*控制高宽比*/
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

/*判断左上坐标并返回*/
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

/*判断左下坐标并返回*/
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

/*判断左下坐标并返回*/
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

/*判断右下坐标并返回*/
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

bool CRCJudge(long i)//判断传输的信息是否有误
{
	long generate = 25;//11001
	char g_add[13];//放被除数多项式
	char s_end[13];
	_itoa(i, s_end, 2);//检验的数
	int len_s = strlen(s_end);
	_itoa(generate, g_add, 2);//CRC多项式
	int len_g = strlen(g_add);
	long temp;
	while (len_s >= len_g)//不断进行异或操作，得到余数
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
				for (int c = 0; c < 84; c++)//行
				{
					for (int t = 0; t < 7; t++)//次
					{
						bool symple = false;
						for (int r = 0; r < 12; r++)//位
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
				Mat srcGray;            //canvas为画布 将找到的定位特征画出来
				Mat canvas = image.clone();
				Mat canvasGray;
				//pyrDown(src, src);  //图片过大时使用
				//canvas = Mat::zeros(image.size(), CV_8UC3);

				/*灰度滤波直方图均值化 提高对比度*/

				cvtColor(image, srcGray, COLOR_BGR2GRAY);
				blur(srcGray, srcGray, Size(3, 3));
				equalizeHist(srcGray, srcGray);

				/*阈值根据实际情况 如视图中已找不到特征 可适量调整*/
				threshold(srcGray, srcGray, 110, 255, THRESH_BINARY);

				/*imshow("test",srcGray);
				waitKey(0);*/

				/*contours是第一次寻找轮廓*/
				/*contours2是筛选出的轮廓*/
				vector<vector<Point>> contours, contours2;
				vector<Vec4i> hierarchy;
				findContours(srcGray, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
				int ic = 0;
				int parentIdx = -1;

				/*个人认为： 下面程序为寻找出有两个子轮廓的父轮廓*/
				/*挺好用的 几乎筛选出来了*/
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

				vector<Point> center_all;  //center_all获取特性中心
				Point2f points[4][4];//记录每张图的定位点坐标
				for (int i = 0, count = 0; i < contours2.size(); i++)
				{
					/*drawContours(canvas, contours2, i, Scalar(0, 255, 0), 1);*/
					double area = contourArea(contours2[i]);
					if (area < 100)
						continue;
					/*控制高宽比*/
					RotatedRect rect = minAreaRect(Mat(contours2[i]));
					double w = rect.size.width;
					double h = rect.size.height;
					double rate = min(w, h) / max(w, h);
					if (rate > 0.85)
					{
						Mat src = transformCorner(image, rect); //返回旋转后的图片
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



				
				//使包围点更贴近定位点
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

				for (int c = 0; c < 84; c++)//行
				{
					for (int t = 0; t < 7; t++)//次
					{
						bool symple = false;
						for (int r = 0; r < 12; r++)//位
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


