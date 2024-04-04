#ifndef IFEATURES_H
#define IFEATURES_H

enum FeatureType {
	FEAT_EDGE_VERT, FEAT_EDGE_HORIZ,
	/* FEAT_LINE_VERT, FEAT_LINE_VERT_THICK, FEAT_LINE_HORIZ, FEAT_LINE_HORIZ_THICK, */
	FEAT_CENTER_SURROUND,

	NUMBER_OF_FEATURES,
	FEAT_FIRST = 0
};

struct PositionedFeature {
	FeatureType feat;
	float sign;
	float x, y, w, h;

	//float fVal; // for learning purposes
};


extern float g_featureValueThreshold;
extern int g_allowedNegatives;


int integralRect(const cv::Mat& ii, int x1, int y1, int x2, int y2);

float evalFeature(const cv::Mat& ii, int x, int y, int w, int h, FeatureType feat, float sign);

bool checkSubwindow(const cv::Mat& ii, const PositionedFeature *features, unsigned N_features,
					int winX, int winY, int winW, int winH);

void searchAndHilightAll(cv::Mat& rgb, const cv::Mat& ii, const PositionedFeature *features, unsigned N_features);


#endif // IFEATURES_H
