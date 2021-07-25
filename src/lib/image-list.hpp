#pragma once

#include <filesystem>
#include <forward_list>
#include <fstream>

#include "../precompiled.h"

#include "detect-edge.hpp"
#include "edged-image.hpp"
#include "image-list.hpp"

class ImageList {
  std::forward_list<EdgedImage> store;
  std::string dirPath;
  int count_;
  bool fromCache_;

  bool getCached();
  void generate();

public:
  ImageList();
  ImageList(std::string dirPath, bool refreshCache = false);

  std::forward_list<EdgedImage>::iterator begin();
  std::forward_list<EdgedImage>::iterator end();

  int count() const;
  bool fromCache() const;
};
