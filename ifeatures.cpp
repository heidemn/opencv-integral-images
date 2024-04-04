#define _CRT_SECURE_NO_WARNINGS
/* #ifdef NDEBUG
#undef NDEBUG
#endif */
#include <cassert>


#include <iostream>
#include <iomanip>
#include <cstdio>
#include <vector>

// Include some OpenCV headers .
// For older versions of OpenCV just include " cv . h " and " highgui . h "
// #include <opencv/cv.h>
// #include <opencv/highgui.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "ifeatures.h"

using namespace std;
using namespace cv;

float g_featureValueThreshold = 14.0f;
int g_allowedNegatives = 1;




int integralRect(const cv::Mat& ii, int x1, int y1, int x2, int y2) {
	return(ii.at<int>(y2,x2) - ii.at<int>(y1,x2) - ii.at<int>(y2,x1) + ii.at<int>(y1,x1));
}


float evalFeature(const cv::Mat& ii, int x, int y, int w, int h, FeatureType feat, float sign) {
	float blackAvg, whiteAvg;
	switch(feat) {
	case FEAT_EDGE_HORIZ:
	{
		int higher_rect_h = h/2;
		int lower_rect_h =  h - higher_rect_h;
		int lower_rect_top = y + higher_rect_h;
		int x2 = x + w;
		whiteAvg = (float)( (double)integralRect(ii,x,y,x2,lower_rect_top) / (double)(w*higher_rect_h) );
		blackAvg = (float)( (double)integralRect(ii,x,lower_rect_top,x2,y+h) / (double)(w*lower_rect_h) );
		break;
	}
	case FEAT_EDGE_VERT:
	{
		int left_rect_w = w/2;
		int right_rect_w = w - left_rect_w;
		int right_rect_x1 = x + left_rect_w;
		int y2 = y + h;
		whiteAvg = (float)( (double)integralRect(ii,x,y,right_rect_x1,y2) / (double)(left_rect_w*h) );
		blackAvg = (float)( (double)integralRect(ii,right_rect_x1,y,x+w,y2) / (double)(right_rect_w*h) );
		break;
	}
	case FEAT_CENTER_SURROUND:
	{
		int w3 = w/3, h3 = h/3;
		int blackSum = integralRect(ii,x+w3,y+h3,x+w3+w3,y+h3+h3);
		blackAvg = (float)( (double)blackSum / (double)(w3*h3) );
		whiteAvg = (float)( (double)(integralRect(ii,x,y,x+w,y+h) - blackSum) / (double)(w*h - w3*h3) );
		break;
	}
	default:
		assert(false);
	}
	return sign * (whiteAvg - blackAvg);
}






bool checkSubwindow(const cv::Mat& ii, const PositionedFeature *features, unsigned N_features,
					int winX, int winY, int winW, int winH) {
	cv::Size size = ii.size();
	int W = size.width;
	int H = size.height;

	assert(winX >= 0);
	assert(winY >= 0);
	assert(winW >= 0);
	assert(winH >= 0);

	//const float THRESHOLD = 15.0f;

	int negatives = 0;
	for(unsigned i = 0; i < N_features; i++) {
		const PositionedFeature *f = &(features[i]);
		int w = (int)(f->w * (float)winW);
		int h = (int)(f->h * (float)winH);
		int x = winX + (int)(f->x * (float)winW);
		int y = winY + (int)(f->y * (float)winH);

		w = min(w, W-x); // nimm x,y als fest und passe evtl w,h an falls zu gross
		h = min(h, H-y);

		float fVal = evalFeature(ii, x, y, w, h, f->feat, f->sign);
		//cout << "Feature " << i << ": value " << fVal << endl;
		if(fVal < g_featureValueThreshold) {
			negatives++;
			if(negatives > g_allowedNegatives)
				return false;
		}
	}
	return true;
}

void searchAndHilightAll(cv::Mat& rgb, const cv::Mat& ii, const PositionedFeature *features, unsigned N_features) {
	Size size = ii.size();
	int W = size.width;
	int H = size.height;

	const float MIN_SIZE_X = 20;
	const float MIN_SIZE_Y = 20;
	const float MAX_SIZE_X = 0.5f * (float)W;
	const float MAX_SIZE_Y = 0.5f * (float)H;
	//float MAX_SIZE = 0.2 * (float)(min(W,H));
	const float MULT = 1.25;

	const float MIN_DELTA_X = 2.0f;
	const float MIN_DELTA_Y = 2.0f;

	//int ALLOWED_NEGATIVES = 2;

	for(float xsize = MIN_SIZE_X; xsize < MAX_SIZE_X; xsize *= MULT) {
		const float xstep = max(0.15f*xsize, MIN_DELTA_X);

		for(float ysize = MIN_SIZE_Y; ysize < MAX_SIZE_Y; ysize *= MULT) {
			const float ystep = max(0.15f*ysize, MIN_DELTA_Y);

			for(float x = 0; x < (float)W - xsize - 1; x += xstep) {
				for(float y = 0; y < (float)H - ysize - 1; y += ystep) {
					if(checkSubwindow(ii, features, N_features,
									  (int)x, (int)y, (int)xsize, (int)ysize))
					{
						Rect r((int)x, (int)y, (int)xsize, (int)ysize);
						cv::rectangle(rgb, r, Scalar(255, 0, 0, 0));
						//cout << "Found: x=" << r.x << " y=" << r.y << " w=" << r.width << " h=" << r.height << endl;
					}
				}
			}
		}
	}
}

