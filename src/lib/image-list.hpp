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
  typedef std::vector<std::shared_ptr<EdgedImage>> image_store;
  typedef std::function<bool(std::shared_ptr<EdgedImage>, std::shared_ptr<EdgedImage>)> sort_predicate;

private:
  std::string dirPath;

  bool getStored();

public:
  image_store store;
  ImageList(std::string dirPath);

  void generate();
  void save();

  void sortBy(const sort_predicate &sortFn);
  void sortBy(const char* sorter);

  image_store::iterator begin();
  image_store::iterator end();
  image_store::reference at(size_t pos);

  int count() const;
};
