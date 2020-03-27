#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include "InformationToImg.h"
#include "ImgToData.h"


#define intest "D:\\桌面\\test\\1.bin"
#define Vediotest "D:\\c++\\ffmpeg\\ConsoleApplication1\\Vedio\\test.mp4"


using namespace std;
using namespace cv;

void ImageToVedio(const char*);
int VedioToImage(const char*);
int VedioToImage_test(const char*);

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

		//DataToImg(intest);

		DataToImg(filename);
		ImageToVedio("QCode");
	}
	else if (choice == "decode")
	{
		cout << "请输入待处理的转码文件的全路径:" << endl;
		char filename[50];
		cin >> filename;
		//if (filename == NULL)
			//VedioToImage(Vediotest);
		//else
			VedioToImage(filename);
		ImageToData("temp");
	}
	else cout << "请输入encode/decode!" << endl;

return 0;
}


/*图片生成视频*/
void ImageToVedio(const char* Path)
{
	VideoWriter video("Vedio\\t.mp4", ('X', 'V', 'I', 'D'), 10.0, Size(1000, 1000));

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
			resize(image, image, Size(1000, 1000));
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
	/*int kernel_size = 3;
	Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F) / (float)(kernel_size * kernel_size);*/

	int count = 0;

	while (capture.read(frame))
	{
		cout << ++count << endl;
		if (IsQr(frame))break;
	}

	//resize(frame, frame, Size(400, 400));
	//imshow("test", frame);
	//waitKey(0);

	count = 0;
	while (!stop)
	{
		//读取下一帧
		if (!capture.read(frame))
		{
			cout << "读取视频失败" << endl;
			return -1;
		}

		//resize(frame, frame, Size(400, 400));
		//imshow("test", frame);
		//waitKey(0);

		if(currentFrame % 6 == 0)
		{
			if (!IsQr(frame))
				break;

			cout << "正在读取第" << currentFrame << "帧" << endl;
			//imshow("Extractedframe", frame);

			cout << "正在写第" << currentFrame << "帧" << endl;
			stringstream str;
			str << ((count++) + 1) << ".png";
			cout << str.str() << endl;
			imwrite("temp\\" + str.str(), frame);
		}


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

int VedioToImage_test(const char* Path)
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
	//namedWindow( "Extractedframe" );

	//两帧间的间隔时间:
	//int delay = 1000/rate;
	double delay = 1000 / rate;


	//利用while循环读取帧
	//currentFrame是在循环体中控制读取到指定的帧后循环结束的变量
	long currentFrame = frameToStart;


	while (!stop)
	{
		//读取下一帧
		if (!capture.read(frame))
		{
			cout << "读取视频失败" << endl;
			return -1;
		}


		//cout << "正在读取第" << currentFrame << "帧" << endl;
		//imshow( "Extractedframe", frame );

		//此处为跳帧操作
		//if (currentFrame % 50 == 0) //此处为帧数间隔，修改这里就可以了
		//{
			cout << "正在写第" << currentFrame << "帧" << endl;
			stringstream str;
			str << (currentFrame + 1) << ".png";
			imwrite("temp_\\" + str.str(), frame);

			cout << str.str() << endl;
			//imwrite(str.str(), frame);
		//}

		//waitKey(intdelay=0)当delay≤ 0时会永远等待；当delay>0时会等待delay毫秒
		//当时间结束前没有按键按下时，返回值为-1；否则返回按键
		int c = waitKey(delay);
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

