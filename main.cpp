#define _CRT_SECURE_NO_WARNINGS
/* #ifdef NDEBUG
#undef NDEBUG
#endif */
#include <cassert>


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <vector>
#include <set>

// Include some OpenCV headers .
// For older versions of OpenCV just include " cv . h " and " highgui . h "
// #include <opencv/cv.h>
// #include <opencv/highgui.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "ifeatures.h"

using namespace cv;
using namespace std;








/*
const int FEAT_REF_W = 398, FEAT_REF_H = 500;

PositionedFeature features[] = {
	// Quadrat links unten
	{ FEAT_EDGE_VERT,  -1, 0, 424, 76*2, 76  },
	{ FEAT_EDGE_HORIZ, +1, 0, 391, 76, 33*2 },
	// Quadrat rechts unten
	{ FEAT_EDGE_VERT,  +1, 322-76, 424, 76*2, 76 },
	{ FEAT_EDGE_HORIZ, +1, 322, 366, 76, 58*2 },

	// langer gerader Teil des Pfeils
	{ FEAT_EDGE_HORIZ, -1, 94, 336, 216, 70 },
	{ FEAT_EDGE_HORIZ, +1, 94, 321, 213, 15*2 },

	// Dose
	{ FEAT_EDGE_VERT,  +1, 94, 199, 47*2, 120 },
	{ FEAT_EDGE_VERT,  -1,191, 199, 15*2, 120 },

	// Dreieck o.l.
	{ FEAT_EDGE_HORIZ, -1,  0, 0, 95, 253 },
	{ FEAT_EDGE_VERT,  -1, 50, 0, 172, 80 },

	// Dreieck o.r.
	{ FEAT_EDGE_HORIZ, -1,288, 0, 110, 220 },
	{ FEAT_EDGE_VERT,  +1,176, 0, 174, 80 }
};*/


/*
bool compareFeatures (const PositionedFeature& f1, const PositionedFeature& f2)
{
	return (f1.fVal > f2.fVal);
}
*/


void learnClass2(vector<PositionedFeature>& features, const cv::Mat& ii, const cv::Mat& mat) 
{
	vector<cv::Scalar> colors;
	// Attention: Integral image is 1 pixel bigger
	assert(ii.size().width  == mat.size().width  + 1);
	assert(ii.size().height == mat.size().height + 1);
	float W = (float)ii.size().width;
	float H = (float)ii.size().height;


	for(int y = 0; y < H-1; y++)	 // Attention: Integral image is 1 pixel bigger
	{
		int foo = 1;
		for(int x = 0; x < W-1; x++) // Attention: Integral image is 1 pixel bigger
		{
			int R,G,B;
			B= mat.at<unsigned char>(y,3*x);
			G= mat.at<unsigned char>(y,3*x+1);
			R= mat.at<unsigned char>(y,3*x+2);
			if((B != G) || (G != R) || (B != R)) {
				Scalar col(R,G,B,0);
				bool colAlreadyUsed = false;
				for(unsigned j = 0; j < colors.size(); j++) {
					if(colors.at(j) == col)
						colAlreadyUsed = true;
				}

				if(!colAlreadyUsed) {
					// (x,y) == top-left vom "Rechteck"
					bool found = false;
					int x2,y2;
					for(y2 = H-2; y2 > y && (!found); y2--) {
						for(x2 = W-2; x2 > x && (!found); x2--) {
							int B2= mat.at<unsigned char>(y2,3*x2);
							int G2= mat.at<unsigned char>(y2,3*x2+1);
							int R2= mat.at<unsigned char>(y2,3*x2+2);
							if(B==B2 && G==G2 && R==R2)
								found = true;
						}
					}
					if(found) {
						const int RADIUS = 3;

						float bestVal = -1, bestSign=0, bestX=x, bestX2=x2, bestY=y, bestY2=y2;
						FeatureType bestFeat = (FeatureType)0;
						/*for(int curX = max(0, x-RADIUS); curX <= min((int)W-1,x+RADIUS); curX++) {
							for(int curY = max(0, y-RADIUS); curY <= min((int)H-1,y+RADIUS); curY++) {
								for(int curX2 = max(curX+1,x2-RADIUS); curX2 <= min((int)W,x2+RADIUS); curX2++) {
									for(int curY2 = max(curY+1,y2-RADIUS); curY2 <= min((int)H,y2+RADIUS); curY2++) {*/
										for(int feat = 0; feat < NUMBER_OF_FEATURES; feat++) {
											float fVal = evalFeature(ii, x, y, x2-x+1, y2-y+1, (FeatureType)feat, 1.0);
											if(fabs(fVal) > bestVal) {
												bestSign = ((fVal > 0.0f) ? 1.0f : (-1.0f));
												bestVal = fabs(fVal);
												bestFeat = (FeatureType)feat;
												/*bestX = curX;
												bestY = curY;
												bestX2 = curX2;
												bestY2 = curY2;*/
											}
										}
									/*}
								}
							}
						}*/

						PositionedFeature f;
						f.feat = bestFeat;
						f.sign = bestSign;
						f.x = (float)bestX / W;
						f.w = (float)(bestX2-bestX+1) / W;
						f.y = (float)bestY / H;
						f.h = (float)(bestY2-bestY+1) / H;
						features.push_back(f);

						cout << "Rectangle " << f.x << "," << f.y << "," << f.w << "," << f.h
							 << "   RGB=" << R << "," << G << "," << B;
						cout << "  feat=" << (int)bestFeat << "  val=" << bestSign*bestVal << endl;

						colors.push_back(col);
					}
				}
			}
		}
	}
}



/*
void learnClassifier(vector<PositionedFeature>& features, int N_features, const cv::Mat& ii) {
	const float VALUE_THRES = 5;

	Size size = ii.size();
	float W = (float)size.width;
	float H = (float)size.height;

	for(float w = max(5.0f, W/25.0f); w < W/4.0f; w *= 1.25f) {
		for(float h = max(5.0f, H/25.0f); h < H/4.0f; h *= 1.25f) {
			for(int x = 0; x <= W - (int)w; x += max(2, (int)(0.25*w))) {
				for(int y = 0; y <= H - (int)h; y += max(2, (int)(0.25*h))) {
					for(int feat = 0; feat < NUMBER_OF_FEATURES; feat++) {
						float sign = 1;
						float fVal = evalFeature(ii, x, y, (int)w, (int)h, (FeatureType)feat, sign);
						if(fVal < 0) {
							fVal = -fVal;
							sign = -sign;
						}

						if(fVal > VALUE_THRES) {
							PositionedFeature f;
							f.feat = (FeatureType)feat;
							f.sign = sign;
							f.x = (float)x / (float)W;
							f.y = (float)y / (float)H;
							f.w = (float)w / (float)W;
							f.h = (float)h / (float)H;
							f.fVal = fVal;
							features.push_back(f);
						}
					}
				}
			}
		}
	}
	cout << "Found " << features.size() << " features.";
	sort(features, compareFeatures);
	features.resize(N_features);
	for(int i = 0; i < N_features; i++) {
		PositionedFeature f = features.at(i);
		cout << "x=" << f.x << " y=" << f.y << " w=" << f.w << " h=" << f.h
			 << " feat=" << f.feat << " sign=" << f.sign << " fVal=" << f.fVal << endl;
	}
}*/



void fileReadError(const char *filename)
{
	std::cout << "Failed to read file: " << filename << std::endl;
	std::exit(1);
}



int main() {
	/*unsigned i;
	for(i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
		PositionedFeature *f = &(features[i]);
		f->x /= (float)FEAT_REF_W;
		f->w /= (float)FEAT_REF_W;
		f->y /= (float)FEAT_REF_H;
		f->h /= (float)FEAT_REF_H;
	}*/


	// Open a window to draw the image . The window title is set to " image ".
    cv::namedWindow("image");

	// Create an image , that will be filled by the camera .
	cv::Mat mat, gray, iimage;

	vector<PositionedFeature> feat;
	cv::Mat rectangles(cv::imread("rectangles.png", 1)); // RGB
	if(rectangles.data == NULL)
		fileReadError("rectangles.png");
	
	gray = cv::imread("pfand2.png", 0);  // gray
	if(gray.data == NULL)		
		fileReadError("pfand2.png");
	if ((gray.size().width  != rectangles.size().width) ||
		(gray.size().height != rectangles.size().height))
	{
		std::cout << "rectangles.png and pfand2.png are not of same size!" << std::endl;
		return 1;
	}

	cv::integral(gray, iimage, CV_32S);
	learnClass2(feat, iimage, rectangles);
	cv::Mat ipfand = iimage;
	//learnClassifier(feat, 25, iimage);

	const PositionedFeature *features = &(feat.at(0));
	int N_features = feat.size();
	cout << "N = " << N_features << endl;

	int nNeg = checkSubwindow(iimage, features, N_features, 0, 0, iimage.size().width,  iimage.size().height);
	cout << "negatives: " << nNeg << endl;





	/*
	gray = cv::imread("pfand.jpg", 0);  // 0==Grayscale
	Size videoSize = gray.size();
	int W = videoSize.width;
	int H = videoSize.height;
	cv::integral(gray, iimage, CV_32S);
	for(i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
		PositionedFeature *f = &(features[i]);
		assert(f->x + f->w <= W);
		assert(f->y + f->h <= H);
		float fVal = evalFeature(iimage, f->x, f->y, f->w, f->h, f->feat, f->sign);
		cout << "Feature(" << setw(2) << i << ") = " << fVal << endl;
	}
	*/




	// Open the camera . Any camera that is supported by OpenCV will be chosen .
    cv::VideoCapture cap(cv::CAP_ANY);

    if(!cap.isOpened())
        cap.open(cv::CAP_ANY);


	/*
	type The type of the matrix elements in the form CV <bit depth><S|U|F>C<number of
	channels>, where S=signed, U=unsigned, F=float. For example, CV 8UC1 means the
	elements are 8-bit unsigned and the there is 1 channel, and CV 32SC2 means the elements
	are 32-bit signed and there are 2 channels.
	*/
	cap >> mat;
	Size videoSize = mat.size();
	int W = videoSize.width;
	int H = videoSize.height;



	double frameRate_timer = (double)getTickCount();
	int frames = 0;
	char fps_s[20] = "? fps";


	// Loop until any key is pressed . During the waiting time ,
	// the image will be drawn to the window .
	int key = -1;
	while(key != 27){
		// Let the camera fill the matrix with the next frame , using
		// the overloaded C ++ streaming operator " > >"

		cap >> mat;
		cv::cvtColor(mat, gray, cv::COLOR_RGB2GRAY, 0 /*auto number of channels*/);
		cv::integral(gray, iimage, CV_32S);


		searchAndHilightAll(mat, iimage, features, N_features);


		frames++;
		// do something ...
		double t = ((double)getTickCount() - frameRate_timer) / getTickFrequency();
		if(t > 1.0) { // after 1 sec...
			int fps = (int)( (double)frames / t );
			sprintf(fps_s, "%3d fps", fps);
			frameRate_timer = (double)getTickCount();
			frames = 0;
		}
		cv::putText(mat, fps_s, Point(10, H-10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255, 0));


		// Paste the image to the window , addressed by the window title " image ".
		//cv::normalize(houghCalc.hough, houghVis, 0.0, 255.0, NORM_MINMAX, CV_8UC1);
		//cv::imshow("hough", houghVis);
		cv::imshow("image", mat);


		/*
		gute Kombinationen:

		g_allowedNegatives = 2, g_featureValueThreshold = 21

		g_allowedNegatives = 1, g_featureValueThreshold = 14.0f

		*/


		key = cv::waitKey(3);
		switch(key & 0xFF) {
		case 't':
			g_featureValueThreshold = max(0.0f, g_featureValueThreshold-1);
			cout << "g_featureValueThreshold = " << g_featureValueThreshold << endl;
			break;
		case 'T':
			g_featureValueThreshold += 1;
			cout << "g_featureValueThreshold = " << g_featureValueThreshold << endl;
			break;
		case 'n':
			g_allowedNegatives = max(0, g_allowedNegatives-1);
			cout << "g_allowedNegatives = " << g_allowedNegatives << endl;
			break;
		case 'N':
			g_allowedNegatives = min(6, g_allowedNegatives+1);
			cout << "g_allowedNegatives = " << g_allowedNegatives << endl;
			break;
		}
		//if(key >= 0)
		//	cout << "keychar: " << (char)key << "  keycode: " << key << endl;
    }


    cv::waitKey(0);
	
	// When the loop is done , close the camera so that the
	// driver is disconnected .
	cap.release () ;

	
	return 0;
}
