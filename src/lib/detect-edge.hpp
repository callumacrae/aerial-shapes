#pragma once

#include "../precompiled.h"
#include "../config.h"

cv::Mat detectEdgesCanny(cv::Mat &sourceImage,
    int blurSize = EDGE_DETECTION_BLUR_SIZE,
    int sigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
    int sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
    int threshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1,
    int threshold2 = EDGE_DETECTION_CANNY_THRESHOLD_1);

boost::dynamic_bitset<unsigned char> edgesToBitset(cv::Mat &edgeMatrix);
