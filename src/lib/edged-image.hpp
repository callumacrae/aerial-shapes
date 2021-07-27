#pragma once

#include <algorithm>

#include "../precompiled.h"
#include "../config.h"

struct ImageMatch {
  float percentage, scale;
  int originX, originY;
};

class EdgedImage {
  using bitset = boost::dynamic_bitset<unsigned char>;

  ImageMatch matchToStep(const cv::Mat &templateImage, float scale, int originX, int originY, float whiteBias = 0.75) const;

  // @todo make this a bit more classey
public:
  std::string path;
  int width, height;
  bitset edges;

  int detectionMode, detectionBlurSize, detectionBlurSigmaX,
      detectionBlurSigmaY, detectionCannyThreshold1, detectionCannyThreshold2;

  EdgedImage() {}
  EdgedImage(std::string path, int width, int height, bitset &edges,
             int detectionMode = ImageEdgeMode_Canny,
             int detectionBlurSize = EDGE_DETECTION_BLUR_SIZE,
             int detectionBlurSigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
             int detectionBlurSigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
             int detectionCannyThreshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1,
             int detectionCannyThreshold2 = EDGE_DETECTION_CANNY_THRESHOLD_2)
      : path(path), width(width), height(height), edges(edges),
        detectionMode(detectionMode), detectionBlurSize(detectionBlurSize),
        detectionBlurSigmaX(detectionBlurSigmaX),
        detectionBlurSigmaY(detectionBlurSigmaY),
        detectionCannyThreshold1(detectionCannyThreshold1),
        detectionCannyThreshold2(detectionCannyThreshold2) {}

  ImageMatch matchTo(const cv::Mat &templateImage, float whiteBias = 0.75) const;
  cv::Mat edgesAsMatrix() const;

  friend std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
};

std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
