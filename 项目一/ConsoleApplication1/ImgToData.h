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

#define w 400;

bool clockwise = 1;

void ImageToData(const char* Path)
{
	String img_path = Path;
	vector<String> img;

	glob(img_path, img, false);

	size_t count = img.size();
	ofstream fout("out.bin", ios::out | ios::binary);
	for (size_t i = 1; i <= count; i++)
	{
		stringstream str;
		str << i << ".png";
		Mat image = imread(img_path + "\\" + str.str());
		cvtColor(image, image, COLOR_RGB2GRAY);

		uchar temp = '\0';
		if (!image.empty())
		{
			if (image.size() == Size(400,400))
			{				
				for (int c = 0; c < 32; c++)//行
				{
					for (int t = 0; t < 4; t++)//次
					{
						
						for (int r = 0; r < 8; r++)//位
						{							
							Scalar color = image.at<uchar>(45 + c * 10, 45 + r * 10 + t * 80);//(y,x)
							cout << color[0];
							if (color[0] != 0)
							{
								switch (r)
								{
								case 0:
									temp += 128;
									break;
								case 1:
									temp += 64;
									break;
								case 2:
									temp += 32;
									break;
								case 3:
									temp += 16;
									break;
								case 4:
									temp += 8;
									break;
								case 5:
									temp += 4;
									break;
								case 6:
									temp += 2;
									break;
								case 7:
									temp += 1;
									break;
								default:
									break;
								}
							}
						}
						fout.write((char*)&temp,sizeof(temp));
						temp = '\0';
					}
				}
			}
			else
			{
				vector<Vec3f> circles;
				HoughCircles(image, circles, HOUGH_GRADIENT, 1, 100, 100, 30, 20, 30);
				cout << circles.size() << endl;
				for (size_t i = 0; i < circles.size(); i++)
				{
					Vec3f tem = circles[i];
					circle(image, Point(tem[0], tem[1]), tem[2],Scalar(0));
					circle(image, Point(tem[0], tem[1]), 2, Scalar(0));
				}
				imshow("test", image);
				waitKey(0);
			}
		}
	}
	fout.close();





	return;
}

double angle(Point pt1, Point pt2, Point pt0)//计算角度,顶点为pt0
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	double angle_line = (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
	if (dx1 * dy2 - dy1 * dx2 > 0) clockwise = 0;
	else clockwise = 1;
	return acos(angle_line) * 180 / 3.141592653;
}

// pointDest存放调整后的三个点，三个点的顺序如下
// pt0----pt1
// 
// pt2
bool AdjustQrPoint(Point* pointSrc, Point* pointDest)
{
	int index1[3] = { 2,1,0 };
	int index2[3] = { 0,2,1 };
	int index3[3] = { 0,1,2 };

	for (int i = 0; i < 3; i++)
	{
		int* n = index1;
		if (i == 0)
			n = index1;
		else if (i == 1)
			n = index2;
		else
			n = index3;

		if (angle(pointSrc[n[0]], pointSrc[n[1]], pointSrc[n[2]]) > 80 && angle(pointSrc[n[0]], pointSrc[n[1]], pointSrc[n[2]]) < 100)
		{
			pointDest[0] = pointSrc[n[2]];
			if (clockwise)
			{
				pointDest[1] = pointSrc[n[0]];
				pointDest[2] = pointSrc[n[1]];
			}
			else
			{
				pointDest[1] = pointSrc[n[1]];
				pointDest[2] = pointSrc[n[0]];
			}
			return true;
		}
	}
	return true;
}




void adjust(Point* pointAdjust,Mat src)
{
	//二维码倾斜角度
	Point hor(pointAdjust[0].x + 300, pointAdjust[0].y);
	double qrAngle = angle(pointAdjust[1], hor, pointAdjust[0]);

	//以二维码左上角点为中心 旋转
	Mat drawingRotation = Mat::zeros(Size(src.cols, src.rows), CV_8UC3);
	double rotationAngle = clockwise ? -qrAngle : qrAngle;
	Mat affine_matrix = getRotationMatrix2D(pointAdjust[0], rotationAngle, 1.0);//求得旋转矩阵
	warpAffine(src, drawingRotation, affine_matrix, drawingRotation.size());
}