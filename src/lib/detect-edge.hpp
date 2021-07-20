#pragma once

#include <boost/dynamic_bitset.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat detectEdges(cv::Mat &sourceImage);
boost::dynamic_bitset<unsigned char> detectEdgesAsBitset(cv::Mat &sourceImage);
