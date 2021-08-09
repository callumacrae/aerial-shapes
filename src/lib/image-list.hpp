#pragma once

#include <atomic>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <thread>
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
  void addFile(const std::filesystem::directory_entry &file);

public:
  image_store store;
  ImageList(std::string dirPath);

  void generate();
  void save();
  int sync();

  int matchTo(const cv::Mat &templateImage, ImageMatch *match,
              EdgedImage **bestMatchImage,
              float offsetScaleStep = MATCH_OFFSET_SCALE_STEP,
              int offsetXStep = MATCH_OFFSET_X_STEP,
              int offsetYStep = MATCH_OFFSET_Y_STEP,
              float minOffsetScale = MATCH_MIN_OFFSET_SCALE,
              int maxOffset = MATCH_MAX_OFFSET,
              float whiteBias = MATCH_WHITE_BIAS);

  void sortBy(const sort_predicate &sortFn);
  void sortBy(const char* sorter);

  image_store::iterator begin();
  image_store::iterator end();
  image_store::reference at(size_t pos);

  void erase(size_t pos);
  int count() const;
};
