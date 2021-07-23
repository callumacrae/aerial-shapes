#include <algorithm>
#include <iostream>
#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

#include "edged-image.hpp"

// whiteBias can be hardcoded once i've figured out what it should be
ImageMatch EdgedImage::matchTo(const cv::Mat &templateImage, float whiteBias) const {
  int channels = templateImage.channels();
  CV_Assert(channels == 1);

  // 300 is the edges width
  // @todo don't hardcode this number
  int sourceImageActualHeight = 300.f / width * height;
  float scaleX = 300.f / templateImage.cols;
  float scaleY = (float) sourceImageActualHeight / templateImage.rows;

  float scaleBase = fmin(scaleX, scaleY);

  ImageMatch bestMatch;

  float offsetScaleStep = 0.1;
  int offsetXStep = 5;
  int offsetYStep = 5;

  float minOffsetScale = 0.4;

  int runs = 0;

  for (float offsetScale = 1; offsetScale >= minOffsetScale; offsetScale -= offsetScaleStep) {
    float scale = scaleBase * offsetScale;

    int originX = 0;
    int originY = 0;

    if (scaleX != 0) {
      originX = (300 - templateImage.cols * scale) / 2;
    }
    if (scaleX != 0) {
      originY = (sourceImageActualHeight - templateImage.rows * scale) / 2;
    }

    int maxOffsetX = std::min(15, originX);
    int maxOffsetY = std::min(15, originY);

    for (int offsetXRoot = 0; offsetXRoot <= maxOffsetX; offsetXRoot += offsetXStep) {
      for (int offsetXMultiplier = -1; offsetXMultiplier <= 1; offsetXMultiplier += 2) {
        // Search inside out, e.g. 0 -1 1 -2 2 -3 3
        int offsetX = offsetXRoot * offsetXMultiplier;

        for (int offsetYRoot = 0; offsetYRoot <= maxOffsetY; offsetYRoot += offsetYStep) {
          for (int offsetYMultiplier = -1; offsetYMultiplier <= 1; offsetYMultiplier += 2) {
            int offsetY = offsetYRoot * offsetYMultiplier;

            ImageMatch match = matchToStep(templateImage, scale, originX + offsetX, originY + offsetY, whiteBias);
            runs++;

            if (match.percentage > bestMatch.percentage) {
              bestMatch = match;
            }

            if (offsetY == 0) {
              break;
            }
          }
        }

        // No need to multiply 0 by both -1 and 1
        if (offsetX == 0) {
          break;
        }
      }
    }
  }

  std::cout << "Runs: " << runs << '\n';

  return bestMatch;
}

ImageMatch EdgedImage::matchToStep(const cv::Mat &templateImage,
    float scale, int originX, int originY, float whiteBias) const {
  int testedBlack = 0;
  int matchingBlack = 0;
  int testedWhite = 0;
  int matchingWhite = 0;

  for (int y = 0; y < templateImage.rows; ++y) {
    const uchar* p = templateImage.ptr<uchar>(y);

    for (int x = 0; x < templateImage.cols; ++x) {
      bool templatePixVal = p[x] != 0;

      int transformedX = originX + floor((float) x * scale);
      int transformedY = originY + floor((float) y * scale);
      int i = transformedY * 300 + transformedX;
      bool sourcePixVal = edges[i];

      if (templatePixVal == 1) {
        if (templatePixVal == sourcePixVal) {
          ++matchingWhite;
        }
        ++testedWhite;
      } else {
        if (templatePixVal == sourcePixVal) {
          ++matchingBlack;
        }
        ++testedBlack;
      }
    }
  }

  float percentageBlack = (float) matchingBlack / testedBlack;
  float percentageWhite = (float) matchingWhite / testedWhite;
  float percentage = percentageWhite * whiteBias + percentageBlack * (1 - whiteBias);
  return ImageMatch{percentage, scale, originX, originY};
}

std::ostream& operator<<(std::ostream& os, const EdgedImage& image) {
    os << image.path << ',' << image.width << ',' << image.height << ','
      << image.edges;
    return os;
}
