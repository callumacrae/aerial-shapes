#pragma once

#include <forward_list>

#include "edged-image.hpp"

class ImageList {
  std::forward_list<EdgedImage> store;
  std::string dirPath;
  int count_;
  bool fromCache_;

  bool getCached();
  void generate();

public:
  ImageList(std::string dirPath, bool refreshCache = false);

  std::forward_list<EdgedImage>::iterator begin();
  std::forward_list<EdgedImage>::iterator end();

  int count() const;
  bool fromCache() const;
};
