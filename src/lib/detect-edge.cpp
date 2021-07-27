#include "detect-edge.hpp"

cv::Mat detectEdges(cv::Mat &sourceImage, int blurSize, int sigmaX, int sigmaY,
    int threshold1, int threshold2) {
  cv::Mat image, imageBlurred, imageCanny, imageDilated;

  int width = EDGE_DETECTION_WIDTH;
  int height = (double) sourceImage.rows / sourceImage.cols * width;
  cv::resize(sourceImage, image, { width, height });

  cv::GaussianBlur(image, imageBlurred, { blurSize, blurSize }, sigmaX, sigmaY);
  cv::Canny(imageBlurred, imageCanny, threshold1, threshold2);

  cv::Mat imageResized;
  if (EDGE_DETECTION_WIDTH == STORED_EDGES_WIDTH) {
    return imageCanny;
  } 

  int finalWidth = STORED_EDGES_WIDTH;
  int finalHeight = (double) sourceImage.rows / sourceImage.cols * finalWidth;

  cv::resize(imageCanny, imageResized, { finalWidth, finalHeight });

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

boost::dynamic_bitset<unsigned char> detectEdgesAsBitset(cv::Mat &sourceImage) {
  cv::Mat edgeMatrix = detectEdges(sourceImage);
  return edgesToBitset(edgeMatrix);
}
