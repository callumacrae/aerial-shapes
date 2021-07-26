#pragma once

#include <algorithm>

#include "../precompiled.h"

#include "../config.h"

struct ImageMatch {
  float percentage, scale;
  int originX, originY;
};

class EdgedImage {
  using bitset = boost::dynamic_bitset<unsigned char>;

  ImageMatch matchToStep(const cv::Mat &templateImage, float scale, int originX, int originY, float whiteBias = 0.75) const;

  // @todo make this a bit more classey
public:
  std::string path;
  int width, height;
  bitset edges;

  EdgedImage() {}
  EdgedImage(std::string path, int width, int height, bitset &edges)
    : path(path), width(width), height(height), edges(edges) {}

  ImageMatch matchTo(const cv::Mat &templateImage, float whiteBias = 0.75) const;
  cv::Mat edgesAsMatrix() const;

  friend std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
};

std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
