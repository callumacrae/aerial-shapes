#include <chrono>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <string>
#include <bitset>

#include <opencv2/core/mat.hpp>

#include "lib/edged-image.hpp"

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  std::forward_list<EdgedImage> imageList;

  std::filesystem::path inputPath { argv[1] };
  inputPath.append(".cache");
  std::ifstream inputFile(inputPath);
  if (!inputFile) {
    std::cerr << "Failed to open cache file.\n";
    exit(1);
  }

  std::string line;
  int count = 0;

  while (std::getline(inputFile, line)) {
    size_t first = line.find(',');
    size_t second = line.find(',', first + 1);
    size_t third = line.find(',', second + 1);

    std::string path = line.substr(0, first);
    int width = std::stoi(line.substr(first + 1, second - first - 1));
    int height = std::stoi(line.substr(second + 1, third - second - 1));
    boost::dynamic_bitset<unsigned char> edges(line.substr(third + 1));

    imageList.emplace_front(path, width, height, edges);
    count++;
  }

  inputFile.close();

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded " << count << " images from cache in " << readElapsed.count() << "s\n";
}
