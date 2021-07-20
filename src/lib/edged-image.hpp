#pragma once

#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

struct EdgedImage {
  using bitset = boost::dynamic_bitset<unsigned char>;

  std::string path;
  int width, height;
  bitset edges;

  EdgedImage(std::string path, int width, int height, bitset edges)
    : path(path), width(width), height(height), edges(edges) {}
  
  friend std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
};

std::ostream& operator<<(std::ostream& os, const EdgedImage& image);
