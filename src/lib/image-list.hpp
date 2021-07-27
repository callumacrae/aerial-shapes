#pragma once

#include <filesystem>
#include <forward_list>
#include <fstream>

#include "../precompiled.h"

#include "detect-edge.hpp"
#include "edged-image.hpp"
#include "image-list.hpp"

class ImageList {
  std::vector<EdgedImage> store;
  std::string dirPath;
  int count_ = 0;

  bool getStored();

public:
  ImageList();
  ImageList(std::string dirPath);

  void generate();
  void save();

  std::vector<EdgedImage>::iterator begin();
  std::vector<EdgedImage>::iterator end();
  std::vector<EdgedImage>::reference at(size_t pos);

  int count() const;
};
