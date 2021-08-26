#pragma once

#include <algorithm>

#include "../precompiled.h"
#include "../config.h"

#include "bitset-serialise.hpp"

struct ImageMatch {
  float percentage = 0, scale = 1;
  int originX = 0, originY = 0;
};

class EdgedImage {
  using bitset = boost::dynamic_bitset<unsigned char>;

  void matchToStep(const cv::Mat &templateImage, const uchar edgesAry[],
                   ImageMatch *match, float scale, int originX, int originY,
                   int rowStep = 1, int colStep = 1,
                   float whiteBias = MATCH_WHITE_BIAS) const;

  cv::Mat originalImage;
  int _matchContextOffsetX;
  int _matchContextOffsetY;

  // @todo make this a bit more classey
public:
  std::string path;
  int width, height;
  bitset edges;

  int detectionMode, detectionBlurSize, detectionBlurSigmaX,
      detectionBlurSigmaY, detectionCannyThreshold1, detectionCannyThreshold2,
      detectionCannyJoinByX, detectionCannyJoinByY,detectionBinaryThreshold;

  ImageMatch lastMatch;

  EdgedImage() {}
  EdgedImage(std::string path, int width, int height, bitset &edges,
             int detectionMode = ImageEdgeMode_Canny,
             int detectionBlurSize = EDGE_DETECTION_BLUR_SIZE,
             int detectionBlurSigmaX = EDGE_DETECTION_BLUR_SIGMA_X,
             int detectionBlurSigmaY = EDGE_DETECTION_BLUR_SIGMA_Y,
             int detectionCannyThreshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1,
             int detectionCannyThreshold2 = EDGE_DETECTION_CANNY_THRESHOLD_2,
             int detectionCannyJoinByX = EDGE_DETECTION_CANNY_JOIN_BY_X,
             int detectionCannyJoinByY = EDGE_DETECTION_CANNY_JOIN_BY_Y,
             int detectionBinaryThreshold = EDGE_DETECTION_BINARY_THRESHOLD)
      : path(path), width(width), height(height), edges(edges),
        detectionMode(detectionMode), detectionBlurSize(detectionBlurSize),
        detectionBlurSigmaX(detectionBlurSigmaX),
        detectionBlurSigmaY(detectionBlurSigmaY),
        detectionCannyThreshold1(detectionCannyThreshold1),
        detectionCannyThreshold2(detectionCannyThreshold2),
        detectionCannyJoinByX(detectionCannyJoinByX),
        detectionCannyJoinByY(detectionCannyJoinByY),
        detectionBinaryThreshold(detectionBinaryThreshold) {}

  void provideMatchContext(int templateOffsetX, int templateOffsetY);
  void resetMatchContext();

  int matchTo(const cv::Mat &templateImage, ImageMatch *match,
              float offsetScaleStep = MATCH_OFFSET_SCALE_STEP,
              int offsetXStep = MATCH_OFFSET_X_STEP,
              int offsetYStep = MATCH_OFFSET_Y_STEP,
              float minOffsetScale = MATCH_MIN_OFFSET_SCALE,
              int maxOffset = MATCH_MAX_OFFSET,
              float whiteBias = MATCH_WHITE_BIAS);
  cv::Mat edgesAsMatrix() const;
  cv::Mat getOriginal(bool cache = true);

  friend std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
};

std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
