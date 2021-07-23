#include <chrono>
#include <iostream>
#include <string>
#include <cmath>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "lib/image-list.hpp"
#include "lib/edged-image.hpp"

#include "config.h"

// todo can we get rid of this pointless initialisation?
static ImageList sourceImages;

static int width = 100;
static int height = 550;
static int lineWidth = 8;
static int whiteBias = 75;

static void redraw(int, void*) {
  cv::Mat canvas = cv::Mat::zeros(CANVAS_HEIGHT, CANVAS_WIDTH, CV_8UC3);

  cv::Point pointA{(CANVAS_WIDTH - width) / 2, (CANVAS_HEIGHT - height) / 2};
  cv::Point pointB{(CANVAS_WIDTH + width) / 2, (CANVAS_HEIGHT + height) / 2};
  cv::rectangle(canvas, pointA, pointB, cv::Scalar(0, 0, 255), lineWidth);

  cv::Mat greyCanvas;
  cv::cvtColor(canvas, greyCanvas, cv::COLOR_BGR2GRAY);

  ImageMatch bestMatch;
  EdgedImage *bestMatchImage = nullptr;

  for (EdgedImage &sourceImage : sourceImages) {
    ImageMatch match = sourceImage.matchTo(greyCanvas, whiteBias / 100.f);
    if (match.percentage > bestMatch.percentage) {
      bestMatch = match;
      bestMatchImage = &sourceImage;
    }
  }

  std::cout << "\nBest match is " << bestMatchImage->path;
  std::cout << "% match: " << (bestMatch.percentage * 100) << "%\n";
  std::cout << "Scale: " << bestMatch.scale << '\n';
  std::cout << "Offset x: " << bestMatch.originX << '\n';
  std::cout << "Offset y: " << bestMatch.originY << "\n\n";

  cv::Mat originalImage = cv::imread(bestMatchImage->path);

  if (originalImage.empty()) {
    std::cerr << "Couldn't read image\n";
    exit(1);
  }

  float realScale = (float) bestMatchImage->width / CACHED_SOURCE_WIDTH;

  cv::Rect roi;
  roi.x = round(bestMatch.originX * realScale);
  roi.y = round(bestMatch.originY * realScale);
  roi.width = round(CANVAS_WIDTH * realScale * bestMatch.scale);
  roi.height = round(CANVAS_HEIGHT * realScale * bestMatch.scale);

  cv::Mat cropped = originalImage(roi);
  cv::Mat scaled;
  cv::resize(cropped, scaled, cv::Size(CANVAS_WIDTH, CANVAS_HEIGHT));

  cv::Mat scaledPlusCanvas, out;
  float canvasAlpha = 0.6;
  cv::bitwise_or(canvas, scaled, scaledPlusCanvas);
  cv::addWeighted(scaledPlusCanvas, canvasAlpha, scaled, 1.f - canvasAlpha, 0, out);

  char text[50];
  sprintf(text, "Percentage match: %.1f%%", bestMatch.percentage * 100);
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  putText(out, text, cv::Point(10, 40), fontFace, 1, cv::Scalar::all(255), 2);

  /* cv::imshow("Canvas", canvas); */
  /* cv::imshow("Cropped", scaled); */
  cv::imshow("Blended", out);
}

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  sourceImages = ImageList(argv[1]);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  if (sourceImages.fromCache()) {
    std::cout << "Loaded " << sourceImages.count() << " images from cache in " <<
      readElapsed.count() << "s\n";
  } else {
    std::cout << "Loaded and cached " << sourceImages.count() << " images in " <<
      readElapsed.count() << "s\n";
  }

  if (argv[2]) {
    cv::Mat templateImage = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);

    if (templateImage.empty()) {
      std::cerr << "Couldn't read template image\n";
      return 1;
    }

    ImageMatch bestMatch;
    EdgedImage *bestMatchImage = nullptr;

    for (EdgedImage &sourceImage : sourceImages) {
      ImageMatch match = sourceImage.matchTo(templateImage);
      std::cout << sourceImage.path << " match: " << (match.percentage * 100) << "%\n";

      if (match.percentage > bestMatch.percentage) {
        bestMatch = match;
        bestMatchImage = &sourceImage;
      }
    }

    std::cout << "\nBest match is " << bestMatchImage->path << " with " << (bestMatch.percentage * 100) << "%\n";

    return 0;
  }

  char sliderWindow[] = "Sliders";
  cv::namedWindow(sliderWindow, cv::WINDOW_AUTOSIZE);
  cv::createTrackbar("Width", sliderWindow, &width, CANVAS_WIDTH + 50, redraw);
  cv::createTrackbar("Height", sliderWindow, &height, CANVAS_HEIGHT + 50, redraw);
  cv::createTrackbar("Line width", sliderWindow, &lineWidth, 50, redraw);
  cv::createTrackbar("White bias * 100", sliderWindow, &whiteBias, 100, redraw);

  redraw(0, 0);

  cv::waitKey(0);
}
