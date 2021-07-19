#include <iostream>
#include <string>
#include <stdio.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char *argv[]) {
  cv::Mat sourceImage = cv::imread("./assets/dev/chris-bair-k87Y12f8LHU-unsplash.jpg");
  if (sourceImage.empty()) {
    std::cerr << "Error: image not found\n";
    return 1;
  }
  cv::Mat image, imageBlurred, imageCanny, imageDilated;

  int width = 300;
  int height = (double) sourceImage.rows / (double) sourceImage.cols * width;
  cv::resize(sourceImage, image, { width, height }, 0, 0);

  int size = 21;
  int sigmaX = 5;
  int sigmaY = 5;

  int threshold1 = 25;
  int threshold2 = 40;

  int trackedSize = size;
  int trackedSigmaX = sigmaX;
  int trackedSigmaY = sigmaY;

  int trackedThreshold1 = threshold1;
  int trackedThreshold2 = threshold2;

  std::string win = "Trackbars";
  cv::namedWindow(win, cv::WINDOW_AUTOSIZE);

  cv::createTrackbar("blur size", win, &trackedSize, 31);
  cv::createTrackbar("blur sigmaX", win, &trackedSigmaX, 30);
  cv::createTrackbar("blur sigmaY", win, &trackedSigmaY, 30);

  cv::createTrackbar("canny threshold 1", win, &trackedThreshold1, 200);
  cv::createTrackbar("canny threshold 2", win, &trackedThreshold2, 200);

  bool changed = true;

  while (true) {
    if (trackedSize % 2 == 0) {
      trackedSize++;
    }

    if (size != trackedSize) {
      changed = true;
      size = trackedSize;
    }

    if (sigmaX != trackedSigmaX) {
      changed = true;
      sigmaX = trackedSigmaX;
    }

    if (sigmaY != trackedSigmaY) {
      changed = true;
      sigmaY = trackedSigmaY;
    }

    if (threshold1 != trackedThreshold1) {
      changed = true;
      threshold1 = trackedThreshold1;
    }

    if (threshold2 != trackedThreshold2) {
      changed = true;
      threshold2 = trackedThreshold2;
    }

    if (changed) {
      cv::GaussianBlur(image, imageBlurred, { size, size }, sigmaX, sigmaY);
      cv::Canny(imageBlurred, imageCanny, threshold1, threshold2);

      cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, {3, 3});
      cv::dilate(imageCanny, imageDilated, kernel);

      cv::imshow("Image", image);
      cv::imshow("Blurred image", imageBlurred);
      cv::imshow("Canny image", imageCanny);
      cv::imshow("Dilated image", imageDilated);

      changed = false;
    }

    cv::waitKey(1);
  }
}
