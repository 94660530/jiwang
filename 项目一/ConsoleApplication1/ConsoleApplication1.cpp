#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>
#include <vector>
//#include <libavutil/log.h>
//#include <libavformat/avformat.h>
#include <stdio.h>
#include "InformationToImg.h"
#include "ImgToData.h"

#define w 400


using namespace std;
using namespace cv;

/*
class CStudent
{
public:
	char szName[20];
	int age;
};
*/

void ImageToVedio(const char*);
int VedioToImage(const char*);

int main()
{
	
	cout << "请选择功能(encode/decode):" << endl;
	string choice;
	cin >> choice;
	if (choice == "encode")
	{
		cout << "请输入待处理的二进制文件的全路径:" << endl;
		char filename[50];
		cin >> filename;
		DataToImg(filename);
		ImageToVedio("QCode");
	}
	else if (choice == "decode")
	{
		cout << "请输入待处理的转码文件的全路径:" << endl;
		char filename[50];
		cin >> filename;
		VedioToImage(filename);
		ImageToData("temp");
	}
	else cout << "请输入encode/decode!" << endl;
	

	//判断边界
	/*
	Mat image_gray = imread("D:\\c++\\ffmpeg\\ConsoleApplication1\\temp\\6.png");
	cvtColor(image_gray, image_gray, COLOR_RGB2GRAY);
	Mat image_binary;
	Rect select;
	
	rectangle(image_gray,
		Point(200, 200),
		Point(600, 600),
		Scalar(255, 255, 255),
		FILLED,
		LINE_8);
	
	adaptiveThreshold(image_gray, image_binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 15, 4.5);
	std::vector< std::vector< cv::Point> > contours;
	cv::imshow("gray image", image_gray);
	cv::imshow("Contours", image_binary);
	waitKey(0);
	cv::findContours(
		image_binary,
		contours,
		noArray(),
		RETR_EXTERNAL,
		CHAIN_APPROX_SIMPLE
	);
	image_binary = Scalar::all(0);
	cv::drawContours(image_binary, contours, -1, Scalar::all(255));
	cout << contours[0];
	select = Rect(int(contours[0][0].x), int(contours[0][0].y), int(contours[0][2].x - contours[0][0].x), int(contours[0][2].y - contours[0][0].y));
	Mat ROI = image_gray(select);
	//Scalar color = image_gray.at<uchar>(5, 5);
	//cout << color[0];
	imshow("test", ROI);
	cv::imshow("gray image", image_gray);
	cv::imshow("Contours", image_binary);

	waitKey(0);
	*/





	/*
	if ((bitset<8>(254)[7]))
	{
		cout << 1;
	}
	*/


	/*
	CStudent s;
	ofstream outFile("students.bin", ios::out | ios::binary);
	while (cin >> s.szName >> s.age)
		outFile.write((char*)&s, sizeof(s));
	outFile.close();
	
	ifstream fin("students.bin", ios::in | ios::binary);
	CStudent t;
	fin.read((char*)&t, sizeof(t));
	fin.close();
	cout << t.szName << ' ' << t.age << endl;	
	return 0;
	*/

return 0;
}

/*
void MyEllipse(Mat img, double angle);
void MyFilledCircle(Mat img, Point center);
void MyPolygon(Mat img);
void MyLine(Mat img, Point start, Point end);
int main(void) {
	char atom_window[] = "Drawing 1: Atom";
	char rook_window[] = "Drawing 2: Rook";
	Mat atom_image = Mat::zeros(w, w, CV_8UC3);
	Mat rook_image = Mat::zeros(w, w, CV_8UC3);
	MyEllipse(atom_image, 90);
	MyEllipse(atom_image, 0);
	MyEllipse(atom_image, 45);
	MyEllipse(atom_image, -45);
	MyFilledCircle(atom_image, Point(w / 2, w / 2));
	MyPolygon(rook_image);
	rectangle(rook_image,
		Point(0, 7 * w / 8),
		Point(w, w),
		Scalar(0, 255, 255),
		FILLED,
		LINE_8);
	MyLine(rook_image, Point(0, 15 * w / 16), Point(w, 15 * w / 16));
	MyLine(rook_image, Point(w / 4, 7 * w / 8), Point(w / 4, w));
	MyLine(rook_image, Point(w / 2, 7 * w / 8), Point(w / 2, w));
	MyLine(rook_image, Point(3 * w / 4, 7 * w / 8), Point(3 * w / 4, w));
	imshow(atom_window, atom_image);
	moveWindow(atom_window, 0, 200);
	imshow(rook_window, rook_image);
	moveWindow(rook_window, w, 200);
	waitKey(0);
	return(0);
}
void MyEllipse(Mat img, double angle)
{
	int thickness = 2;
	int lineType = 8;
	ellipse(img,
		Point(w / 2, w / 2),
		Size(w / 4, w / 16),
		angle,
		0,
		360,
		Scalar(255, 0, 0),
		thickness,
		lineType);
}
void MyFilledCircle(Mat img, Point center)
{
	circle(img,
		center,
		w / 32,
		Scalar(0, 0, 255),
		FILLED,
		LINE_8);
}
void MyPolygon(Mat img)
{
	int lineType = LINE_8;
	Point rook_points[1][20];
	rook_points[0][0] = Point(w / 4, 7 * w / 8);
	rook_points[0][1] = Point(3 * w / 4, 7 * w / 8);
	rook_points[0][2] = Point(3 * w / 4, 13 * w / 16);
	rook_points[0][3] = Point(11 * w / 16, 13 * w / 16);
	rook_points[0][4] = Point(19 * w / 32, 3 * w / 8);
	rook_points[0][5] = Point(3 * w / 4, 3 * w / 8);
	rook_points[0][6] = Point(3 * w / 4, w / 8);
	rook_points[0][7] = Point(26 * w / 40, w / 8);
	rook_points[0][8] = Point(26 * w / 40, w / 4);
	rook_points[0][9] = Point(22 * w / 40, w / 4);
	rook_points[0][10] = Point(22 * w / 40, w / 8);
	rook_points[0][11] = Point(18 * w / 40, w / 8);
	rook_points[0][12] = Point(18 * w / 40, w / 4);
	rook_points[0][13] = Point(14 * w / 40, w / 4);
	rook_points[0][14] = Point(14 * w / 40, w / 8);
	rook_points[0][15] = Point(w / 4, w / 8);
	rook_points[0][16] = Point(w / 4, 3 * w / 8);
	rook_points[0][17] = Point(13 * w / 32, 3 * w / 8);
	rook_points[0][18] = Point(5 * w / 16, 13 * w / 16);
	rook_points[0][19] = Point(w / 4, 13 * w / 16);
	const Point* ppt[1] = { rook_points[0] };
	int npt[] = { 20 };
	fillPoly(img,
		ppt,
		npt,
		1,
		Scalar(255, 255, 255),
		lineType);
}
void MyLine(Mat img, Point start, Point end)
{
	int thickness = 2;
	int lineType = LINE_8;
	line(img,
		start,
		end,
		Scalar(0, 0, 0),
		thickness,
		lineType);
}
*/


/*图片生成视频*/
void ImageToVedio(const char* Path)
{
	VideoWriter video("Vedio\\test.mp4", ('X', 'V', 'I', 'D'), 5.0, Size(w, w));

	String img_path = Path;
	vector<String> img;

	glob(img_path, img, false);

	size_t count = img.size();
	for (size_t i = 1; i <= count; i++)
	{
		stringstream str;
		str << i << ".png";
		Mat image = imread(img_path + "\\" + str.str());
		//cout << img_path + "\\" + str.str();
		if (!image.empty())
		{
			resize(image, image, Size(w, w));
			video << image;
			cout << "正在处理第" << i << "帧" << endl;
		}
	}
	cout << "处理完毕！" << endl;
}


/*视频转图片*/
int VedioToImage(const char* Path)
{
	//打开视频文件：其实就是建立一个VideoCapture结构
	VideoCapture capture(Path);
	//检测是否正常打开:成功打开时，isOpened返回ture
	if (!capture.isOpened())
		cout << "fail toopen!" << endl;

	//获取整个帧数
	long totalFrameNumber = capture.get(CAP_PROP_FRAME_COUNT);
	cout << "整个视频共" << totalFrameNumber << "帧" << endl;
	//设置开始帧()
	long frameToStart = 0;
	capture.set(CAP_PROP_POS_FRAMES, frameToStart);
	cout << "从第" << frameToStart << "帧开始读" << endl;


	//设置结束帧
	int frameToStop = INT16_MAX;

	if (frameToStop < frameToStart)
	{
		cout << "结束帧小于开始帧，程序错误，即将退出！" << endl;
		return -1;
	}
	else
	{
		cout << "结束帧为：第" << frameToStop << "帧" << endl;
	}

	//获取帧率
	double rate = capture.get(CAP_PROP_FPS);
	cout << "帧率为:" << rate << endl;
	//定义一个用来控制读取视频循环结束的变量
	bool stop = false;

	//承载每一帧的图像
	Mat frame;

	//显示每一帧的窗口
	//namedWindow("Extractedframe");

	//两帧间的间隔时间:
	//int delay = 1000/rate;
	double delay = 1000 / rate;


	//利用while循环读取帧
	//currentFrame是在循环体中控制读取到指定的帧后循环结束的变量
	long currentFrame = frameToStart;


	//滤波器的核
	int kernel_size = 3;
	Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F) / (float)(kernel_size * kernel_size);

	while (!stop)
	{
		//读取下一帧
		if (!capture.read(frame))
		{
			cout << "读取视频失败" << endl;
			return -1;
		}


		cout << "正在读取第" << currentFrame << "帧" << endl;
		//imshow("Extractedframe", frame);

		cout << "正在写第" << currentFrame << "帧" << endl;
		stringstream str;
		str << (currentFrame + 1) << ".png";
		cout << str.str() << endl;
		imwrite("temp\\" + str.str(), frame);

		//waitKey(intdelay=0)当delay≤ 0时会永远等待；当delay>0时会等待delay毫秒
		//当时间结束前没有按键按下时，返回值为-1；否则返回按键
		//int c = waitKey(delay);
		int c = waitKey(1000);
		//按下ESC或者到达指定的结束帧后退出读取视频
		if ((char)c == 27 || currentFrame > frameToStop)
		{
			stop = true;
		}
		//按下按键后会停留在当前帧，等待下一次按键
		if (c >= 0)
		{
			waitKey(0);
		}
		currentFrame++;

	}
	//关闭视频文件
	capture.release();
	waitKey(0);
	return 0;
}