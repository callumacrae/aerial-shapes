#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

#include "edged-image.hpp"

// whiteBias can be hardcoded once i've figured out what it should be
ImageMatch EdgedImage::matchTo(const cv::Mat &templateImage, float whiteBias) const {
  int channels = templateImage.channels();
  CV_Assert(channels == 1);

  int testedBlack = 0;
  int matchingBlack = 0;
  int testedWhite = 0;
  int matchingWhite = 0;

  // 300 is the edges width
  // @todo don't hardcode this number
  // @todo simplify
  int sourceImageActualHeight = 300.f / width * height;
  float scaleX = 300.f / templateImage.cols;
  float scaleY = (float) sourceImageActualHeight / templateImage.rows;

  float scale = fmin(scaleX, scaleY);

  int originX = 0;
  int originY = 0;

  if (scale == scaleX) {
    originY = (sourceImageActualHeight - templateImage.rows * scale) / 2;
  } else {
    originX = (300 - templateImage.cols * scale) / 2;
  }

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

  // @todo probably shouldn't treat matching black + white the same
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
