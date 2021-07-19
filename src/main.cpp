#include <iostream>
#include <string>
#include <stdio.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "lib/detect-edge.hpp"

int main(int argc, const char *argv[]) {
  cv::Mat sourceImage = cv::imread("./assets/dev/chris-bair-k87Y12f8LHU-unsplash.jpg");
  if (sourceImage.empty()) {
    std::cerr << "Error: image not found\n";
    return 1;
  }

  /* cv::imshow("Image", sourceImage); */

  std::vector<bool> bitmap = detectEdges(sourceImage);

  for (bool p : bitmap) {
    std::cout << p << ',';
  }
}
