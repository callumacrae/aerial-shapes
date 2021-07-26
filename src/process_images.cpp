#include "linenoise.hpp"

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

  if (!argv[1]) {
    std::cerr << "No directory specified, exiting\n";
    exit(1);
  }

  ImageList imageList(argv[1]);

  if (imageList.fromCache()) {
    auto readFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> readElapsed = readFinish - readStart;

    printf("Loaded %d images from store in %.2fs\n", imageList.count(),
        readElapsed.count());
  } else {
    std::cout << "No store found for this directory, type \"generate\" to " <<
      "generate one\n";
  }

  while (true) {
    std::string line;
    auto quit = linenoise::Readline("\n> ", line);

    if (quit) {
      break;
    }

    std::string_view query(line);
    auto splitPos = query.find(' ');
    std::string_view command = query.substr(0, splitPos);
    std::string_view arg;

    if (splitPos != query.npos) {
      arg = query.substr(query.find(' ') + 1);
    }

    linenoise::AddHistory(line.c_str());

    if (command == "exit" || command == "q") {
      std::cout << "bye\n";
      break;
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (command == "generate") {
      imageList.generate();

      auto finish = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> elapsed = finish - start;
      printf("Generated in %.2fs\n", elapsed.count());
    } else if (command == "save") {
      imageList.save();

      auto finish = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> elapsed = finish - start;
      printf("Saved in %.2fs\n", elapsed.count());
    } else if (command == "ls") {
      printf("%d items in store:\n\n", imageList.count());

      int i = 0;
      for (const EdgedImage &image : imageList) {
        printf("#%d: %s (%dx%d)\n", i, image.path.c_str(), image.width, image.height);
        i++;
      }

      std::cout << '\n';
    } else if (command == "preview") {
      if (arg.empty()) {
        std::cerr << "preview needs argument\n";
        continue;
      }

      EdgedImage image;
      if (arg.find('/') != arg.npos) {
      } else {
        int id = stoi(std::string(arg));
        image = imageList.at(id);
      }

      std::cout << "Previewing: " << image.path << "\n";

      sourceImage = cv::imread(image.path);

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
      cv::destroyAllWindows();
      cv::waitKey(1); // destroyAllWindows doesn't work without this
    }
  }
}
