#include <chrono>
#include <iostream>
#include <string>

#include "lib/detect-edge.hpp"
#include "lib/image-list.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  if (strcmp(argv[1], "--preview") == 0) {
    cv::Mat sourceImage = cv::imread(argv[2]);

    if (sourceImage.empty()) {
      std::cerr << "Could not read source image\n";
      return 1;
    }

    cv::Mat imageEdges = detectEdges(sourceImage);
    cv::imshow("Preview", imageEdges);
    cv::waitKey(0);
    return 0;
  }

  ImageList imageList(argv[1], true);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded and cached " << imageList.count() << " images in " <<
    readElapsed.count() << "s\n";
}
