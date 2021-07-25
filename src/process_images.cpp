#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/image-list.hpp"

static int blurSize = EDGE_DETECTION_BLUR_SIZE;
static int sigmaX = EDGE_DETECTION_BLUR_SIGMA_X;
static int sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y;
static int threshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1;
static int threshold2 = EDGE_DETECTION_CANNY_THRESHOLD_2;

static cv::Mat sourceImage;

static void redrawPreview(int, void*) {
  if (blurSize % 2 == 0) {
    blurSize++;
  }

  cv::Mat greyEdges = detectEdges(sourceImage, blurSize, sigmaX, sigmaY,
      threshold1, threshold2);
  cv::Mat dilatedEdges, edges, scaledEdges, scaledPlusEdges;
  cv::cvtColor(greyEdges, edges, cv::COLOR_GRAY2BGR);

  cv::Mat mask;
  cv::threshold(greyEdges, mask, 0, 255, cv::THRESH_BINARY);
  edges.setTo(cv::Scalar(255, 0, 0), mask);

  cv::resize(edges, scaledEdges, sourceImage.size());
  cv::bitwise_or(scaledEdges, sourceImage, scaledPlusEdges);

  cv::imshow("Preview", scaledPlusEdges);
}

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();
  
  if (strcmp(argv[1], "--preview") == 0) {
    sourceImage = cv::imread(argv[2]);

    if (sourceImage.empty()) {
      std::cerr << "Could not read source image\n";
      return 1;
    }

    char sliderWindow[] = "Sliders";
    cv::namedWindow(sliderWindow, cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Blur size", sliderWindow, &blurSize, 50, redrawPreview);
    cv::createTrackbar("Sigma X", sliderWindow, &sigmaX, 20, redrawPreview);
    cv::createTrackbar("Sigma Y", sliderWindow, &sigmaY, 20, redrawPreview);
    cv::createTrackbar("Threshold 1", sliderWindow, &threshold1, 50, redrawPreview);
    cv::createTrackbar("Threshold 2", sliderWindow, &threshold2, 50, redrawPreview);

    redrawPreview(0, 0);

    cv::waitKey(0);
    return 0;
  }

  ImageList imageList(argv[1], true);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded and cached " << imageList.count() << " images in " <<
    readElapsed.count() << "s\n";
}
