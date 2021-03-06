//
//  main.cpp
//  Shape detection
//
//  Created by Simarpreet Singh Chawla on 10/04/14.
//  Copyright (c) 2014 Simarpreet Singh Chawla. All rights reserved.
//

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src;
Mat src_gray;

static double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void setLabel(Mat& im, const string label, vector<Point>& contour)
{
	int fontface = FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;
    
	Size text = getTextSize(label, fontface, scale, thickness, &baseline);
	Rect r = boundingRect(contour);
	Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	rectangle(im, pt + Point(0, baseline), pt + Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
	putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

int main(int argc, const char * argv[])
{
    src = imread("/Users/Sam/Desktop/3.jpg",CV_LOAD_IMAGE_UNCHANGED);
    cvtColor(src, src_gray, CV_BGR2GRAY);
    Mat canny_output;
    Canny(src_gray, canny_output, 0, 50, 5);
    vector<vector<Point> > contours;
    findContours(canny_output.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    vector<Point> approx;
    Mat dst = src.clone();
    for (int i = 0; i < contours.size(); i++)
    {
        approxPolyDP(contours[i], approx, arcLength(Mat(contours[i]), true) * 0.02, true);
        if (fabs(contourArea(contours[i])) < 100 || !isContourConvex(approx)) {
            continue;
        }
        if (approx.size() == 3)
            setLabel(dst, "TRI", contours[i]);
        else if (approx.size() >= 4 && approx.size() <= 6)
        {
            unsigned long int vtc = approx.size();
            vector<double> cos;
            for (int j = 2; j < vtc+1; j++)
            {
                cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));
            }
            sort(cos.begin(), cos.end());
            double mincos = cos.front();
            double maxcos = cos.back();
            if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
            {
                Rect r = boundingRect(contours[i]);
                double ratio = abs(1 - (double)r.width / r.height);
                setLabel(dst, ratio <= 0.02 ? "SQU" : "RECT", contours[i]);
            }
            else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
                setLabel(dst, "PENTA", contours[i]);
            else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
                setLabel(dst, "HEXA", contours[i]);
        }
        else
        {
            double area = contourArea(contours[i]);
            Rect r = boundingRect(contours[i]);
            int radius = r.width / 2;
            
            if (abs(1 - ((double)r.width / r.height)) <= 0.2 &&
                abs(1 - (area / (CV_PI * pow(radius, 2)))) <= 0.2)
            {
                setLabel(dst, "CIR", contours[i]);
            }
        }
    }
    imshow("src", src);
    imshow("dst", dst);
    waitKey(0);
    return 0;
}

