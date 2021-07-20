#include <chrono>
#include <iostream>
#include <string>

#include "lib/image-list.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList imageList(argv[1], true);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded and cached " << imageList.count() << " images in " <<
    readElapsed.count() << "s\n";
}
