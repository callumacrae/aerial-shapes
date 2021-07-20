#pragma once

#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

using bitset = boost::dynamic_bitset<unsigned char>;

struct EdgedImage {
  std::string path;
  int width, height;
  bitset edges;

  EdgedImage(std::string path, int width, int height, bitset edges)
    : path(path), width(width), height(height), edges(edges) {}
};

std::ostream& operator<<(std::ostream& os, const EdgedImage& image)
{
    os << image.path << ',' << image.width << ',' << image.height << ','
      << image.edges;
    return os;
}
