#include <chrono>
#include <iostream>
#include <string>
#include <cmath>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "lib/image-list.hpp"
#include "lib/edged-image.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList sourceImages(argv[1]);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  if (sourceImages.fromCache()) {
    std::cout << "Loaded " << sourceImages.count() << " images from cache in " <<
      readElapsed.count() << "s\n";
  } else {
    std::cout << "Loaded and cached " << sourceImages.count() << " images in " <<
      readElapsed.count() << "s\n";
  }

  cv::Mat templateImage = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);

  if (templateImage.empty()) {
    std::cerr << "Couldn't read template image\n";
    return 1;
  }

  int channels = templateImage.channels();
  CV_Assert(channels == 1);

  int templateRows = templateImage.rows;
  int templateCols = templateImage.cols;

  for (const EdgedImage &sourceImage : sourceImages) {
    int tested = 0;
    int matching = 0;

    // 300 is the edges width
    // @todo don't hardcode this number
    // @todo simplify
    int sourceImageActualHeight = 300.f / sourceImage.width * sourceImage.height;
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

    for (int y = 0; y < templateRows; ++y) {
      uchar* p = templateImage.ptr<uchar>(y);

      for (int x = 0; x < templateCols; ++x) {
        bool templatePixVal = p[x] != 0;

        int transformedX = originX + round((float) x * scale);
        int transformedY = originY + round((float) y * scale);
        int i = transformedY * 300 + transformedX;
        bool sourcePixVal = sourceImage.edges[i];
        if (templatePixVal == sourcePixVal) {
          ++matching;
        }
        ++tested;
      }
    }

    // @todo simplify
    float matchPercentage = 1.f / tested * matching;
    std::cout << sourceImage.path << " match: " << (matchPercentage * 100) << "%\n";

    /* break; */
  }
}
