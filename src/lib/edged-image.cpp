#include <string>

#include <boost/dynamic_bitset.hpp>
#include <opencv2/core/mat.hpp>

#include "edged-image.hpp"

std::ostream& operator<<(std::ostream& os, const EdgedImage& image) {
    os << image.path << ',' << image.width << ',' << image.height << ','
      << image.edges;
    return os;
}
