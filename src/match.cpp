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

  for (const EdgedImage &sourceImage : sourceImages) {
    ImageMatch match = sourceImage.matchTo(templateImage);
    std::cout << sourceImage.path << " match: " << (match.percentage * 100) << "%\n";
  }
}
