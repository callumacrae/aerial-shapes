#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

struct EdgedImage {
  std::string path;
  int width, height;
  boost::dynamic_bitset<> edges;

  EdgedImage(std::string path, cv::Mat image, boost::dynamic_bitset<> edges)
    : path(path), width(image.rows), height(image.cols), edges(edges) {}
};
