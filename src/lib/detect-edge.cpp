#include "detect-edge.hpp"

static cv::Mat blurImage(cv::Mat &sourceImage, int blurSize, int sigmaX,
                         int sigmaY) {
  cv::Mat image, imageBlurred;

  int width = EDGE_DETECTION_WIDTH;
  int height = (double) sourceImage.rows / sourceImage.cols * width;
  cv::resize(sourceImage, image, { width, height });
  cv::GaussianBlur(image, imageBlurred, { blurSize, blurSize }, sigmaX, sigmaY);

  return imageBlurred;
}

cv::Mat detectEdgesCanny(cv::Mat &sourceImage, int blurSize, int sigmaX,
                         int sigmaY, int threshold1, int threshold2,
                         int joinByX, int joinByY) {
  cv::Mat imageBlurred = blurImage(sourceImage, blurSize, sigmaX, sigmaY);
  cv::Mat imageCanny;

  cv::Canny(imageBlurred, imageCanny, threshold1, threshold2);

  if (joinByX != 0 || joinByY != 0) {
    if (joinByX == 0) {
      joinByX = 1;
    } else if (joinByY == 0) {
      joinByY = 1;
    }

    cv::Mat imageDilated;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                       cv::Size(joinByX, joinByY));

    cv::dilate(imageCanny, imageDilated, kernel);
    cv::erode(imageDilated, imageCanny, kernel);
  }

  if (EDGE_DETECTION_WIDTH == STORED_EDGES_WIDTH) {
    return imageCanny;
  } 

  cv::Mat imageResized;
  int finalWidth = STORED_EDGES_WIDTH;
  int finalHeight = (double) sourceImage.rows / sourceImage.cols * finalWidth;

  cv::resize(imageCanny, imageResized, { finalWidth, finalHeight });

  return imageResized;
}

cv::Mat detectEdgesThreshold(cv::Mat &sourceImage, int blurSize, int sigmaX,
                             int sigmaY, int binaryThreshold) {
  cv::Mat imageBlurred = blurImage(sourceImage, blurSize, sigmaX, sigmaY);
  cv::Mat imageGray, imageThreshold;

  cv::cvtColor(imageBlurred, imageGray, cv::COLOR_BGR2GRAY);
  cv::threshold(imageGray, imageThreshold, binaryThreshold, 255,
                cv::THRESH_BINARY);

  cv::Mat imageResized;
  if (EDGE_DETECTION_WIDTH == STORED_EDGES_WIDTH) {
    imageResized = imageThreshold;
  } 

  int finalWidth = STORED_EDGES_WIDTH;
  int finalHeight = (double) sourceImage.rows / sourceImage.cols * finalWidth;

  cv::resize(imageThreshold, imageResized, { finalWidth, finalHeight });

  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;

  cv::findContours(imageResized, contours, hierarchy, cv::RETR_TREE,
                   cv::CHAIN_APPROX_NONE);

  imageResized.setTo(cv::Scalar::all(0));
  cv::drawContours(imageResized, contours, -1, cv::Scalar(255));

  return imageResized;
}

boost::dynamic_bitset<unsigned char> edgesToBitset(cv::Mat &edgeMatrix) {
  int channels = edgeMatrix.channels();
  CV_Assert(channels == 1);

  int nRows = edgeMatrix.rows;
  int nCols = edgeMatrix.cols;

  if (edgeMatrix.isContinuous()) {
    nCols *= nRows;
    nRows = 1;
  }

  boost::dynamic_bitset<unsigned char> bitset(nRows * nCols);

  int i = 0;
  for (int y = 0; y < nRows; ++y) {
    uchar* p = edgeMatrix.ptr<uchar>(y);
    
    for (int x = 0; x < nCols; ++x) {
      bitset[i] = p[x] != 0;
      ++i;
    }
  }

  return bitset;
}
