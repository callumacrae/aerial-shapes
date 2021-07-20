#include <chrono>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

#include "lib/detect-edge.hpp"
#include "lib/edged-image.hpp"

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  std::forward_list<EdgedImage> imageList;

  if (!fs::exists(argv[1])) {
    std::cerr << "Directory doesn't exist: " << argv[1] << '\n';
    exit(1);
  }

  for (const auto &file : fs::directory_iterator(argv[1])) {
    if (file.path().filename() == ".cache") {
      std::cout << "Skipping: .cache\n";
      continue;
    }

    std::cout << "Reading: " << file.path() << '\n';

    cv::Mat sourceImage = cv::imread(file.path());
    auto edges = detectEdgesAsBitset(sourceImage);
    imageList.emplace_front(file.path(), sourceImage, edges);
  }

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Images loaded in " << readElapsed.count() << "s\n";

  auto writeStart = std::chrono::high_resolution_clock::now();

  std::filesystem::path outputPath { argv[1] };
  outputPath.append(".cache");
  std::ofstream outputFile(outputPath);
  if (!outputFile) {
    std::cerr << "Failed to open cache file.\n";
    exit(1);
  }

  for (const EdgedImage &image : imageList) {
    outputFile << image << '\n';
  }

  auto writeFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> writeElapsed = writeFinish - writeStart;

  std::cout << "Cache file written in " << writeElapsed.count() << "s\n";
}
