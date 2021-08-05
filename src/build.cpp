#include "precompiled.h"

#include "config.h"

#include "lib/frame-collection.hpp"

int main(int argc, const char *argv[]) {
  std::string name(argv[1]);
  FrameCollection frames(name);

  std::cout << "Built: " << frames.size() << '\n';
}
