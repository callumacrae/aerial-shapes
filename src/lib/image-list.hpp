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
  bool fromCache_ = false;

  bool getCached();

public:
  ImageList();
  ImageList(std::string dirPath);

  void generate();
  void save();

  std::vector<EdgedImage>::iterator begin();
  std::vector<EdgedImage>::iterator end();
  EdgedImage at(size_t pos);

  int count() const;
  bool fromCache() const;
};
