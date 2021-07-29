#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include "../precompiled.h"

#include "detect-edge.hpp"
#include "edged-image.hpp"
#include "image-list.hpp"

class ImageList {
public:
  typedef std::vector<std::shared_ptr<EdgedImage>> ImageStore;

private:
  ImageStore store;
  std::string dirPath;

  bool getStored();

public:
  ImageList(std::string dirPath);

  void generate();
  void save();

  ImageStore::iterator begin();
  ImageStore::iterator end();
  ImageStore::reference at(size_t pos);

  int count() const;
};
