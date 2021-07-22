#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

#include "edged-image.hpp"

ImageMatch EdgedImage::matchTo(cv::Mat &templateImage) {
  int channels = templateImage.channels();
  CV_Assert(channels == 1);

  int tested = 0;
  int matching = 0;

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
    uchar* p = templateImage.ptr<uchar>(y);

    for (int x = 0; x < templateImage.cols; ++x) {
      bool templatePixVal = p[x] != 0;

      int transformedX = originX + round((float) x * scale);
      int transformedY = originY + round((float) y * scale);
      int i = transformedY * 300 + transformedX;
      bool sourcePixVal = edges[i];
      if (templatePixVal == sourcePixVal) {
        ++matching;
      }
      ++tested;
    }
  }

  float percentage = (float) matching / tested;
  return ImageMatch{percentage, scale, originX, originY};
}

std::ostream& operator<<(std::ostream& os, const EdgedImage& image) {
    os << image.path << ',' << image.width << ',' << image.height << ','
      << image.edges;
    return os;
}
