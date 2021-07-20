#include <chrono>
#include <iostream>
#include <string>

#include "lib/image-list.hpp"
#include "lib/edged-image.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList imageList(argv[1]);

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded " << imageList.count() << " images" <<
    (imageList.fromCache() ? " from cache" : "") << " in " <<
    readElapsed.count() << "s\n";
}
