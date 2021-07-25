#pragma once

#include <boost/dynamic_bitset.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../config.h"

cv::Mat detectEdges(cv::Mat &sourceImage,
    int blurSize = EDGE_DETECTION_BLUR_SIZE,
    int sigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
    int sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
    int threshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1,
    int threshold2 = EDGE_DETECTION_CANNY_THRESHOLD_1);
boost::dynamic_bitset<unsigned char> detectEdgesAsBitset(cv::Mat &sourceImage);
