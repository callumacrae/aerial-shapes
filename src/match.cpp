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

  if (imageList.fromCache()) {
    std::cout << "Loaded " << imageList.count() << " images from cache in " <<
      readElapsed.count() << "s\n";
  } else {
    std::cout << "Loaded and cached " << imageList.count() << " images in " <<
      readElapsed.count() << "s\n";
  }
}
