#pragma once

#include "../precompiled.h"
#include "../config.h"

cv::Mat detectEdgesCanny(const cv::Mat &sourceImage,
    int blurSize = EDGE_DETECTION_BLUR_SIZE,
    int sigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
    int sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
    int threshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1,
    int threshold2 = EDGE_DETECTION_CANNY_THRESHOLD_1,
    int joinByX = EDGE_DETECTION_CANNY_JOIN_BY_X,
    int joinByY = EDGE_DETECTION_CANNY_JOIN_BY_Y);

cv::Mat detectEdgesThreshold(const cv::Mat &sourceImage,
    int blurSize = EDGE_DETECTION_BLUR_SIZE,
    int sigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
    int sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
    int binaryThreshold = EDGE_DETECTION_BINARY_THRESHOLD);

boost::dynamic_bitset<unsigned char> edgesToBitset(cv::Mat &edgeMatrix);
