#include "image-list.hpp"

ImageList::ImageList(std::string dirPath) : dirPath(dirPath) {
  namespace fs = std::filesystem;

  if (!fs::exists(dirPath)) {
    // @todo probably shouldn't log + exit from here
    std::cerr << "Directory doesn't exist: " << dirPath << '\n';
    exit(1);
  }

  getStored();
}

bool ImageList::getStored() {
  namespace fs = std::filesystem;

  fs::path storePath { dirPath };
  storePath.append(".store");
  std::ifstream storeFile(storePath);
  if (!storeFile) {
    return false;
  }

  std::string line;

  while (std::getline(storeFile, line)) {
    std::string path;
    int width, height;
    boost::dynamic_bitset<unsigned char> edges;

    int detectionMode, detectionBlurSize, detectionBlurSigmaX,
        detectionBlurSigmaY, detectionCannyThreshold1, detectionCannyThreshold2,
        detectionCannyJoinByX, detectionCannyJoinByY, detectionBinaryThreshold;

    std::stringstream lineStream(line);
    int i = 0;
    while (lineStream.good()) {
      std::string substr;
      getline(lineStream, substr, ',');

      if (i == 0) {
        path = substr;
      } else if (i == 1) {
        width = std::stoi(substr);
      } else if (i == 2) {
        height = std::stoi(substr);
      } else if (i == 3) {
        edges = boost::dynamic_bitset<unsigned char>(substr);
      } else if (i == 4) {
        detectionMode = std::stoi(substr);
      } else if (i == 5) {
        detectionBlurSize = std::stoi(substr);
      } else if (i == 6) {
        detectionBlurSigmaX = std::stoi(substr);
      } else if (i == 7) {
        detectionBlurSigmaY = std::stoi(substr);
      } else if (i == 8) {
        detectionCannyThreshold1 = std::stoi(substr);
      } else if (i == 9) {
        detectionCannyThreshold2 = std::stoi(substr);
      } else if (i == 10) {
        detectionBinaryThreshold = std::stoi(substr);
      } else if (i == 11) {
        detectionCannyJoinByX = std::stoi(substr);
      } else if (i == 12) {
        detectionCannyJoinByY = std::stoi(substr);
      }

      ++i;
    }

    store.push_back(std::make_shared<EdgedImage>(
        path, width, height, edges, detectionMode, detectionBlurSize,
        detectionBlurSigmaX, detectionBlurSigmaY, detectionCannyThreshold1,
        detectionCannyThreshold2, detectionCannyJoinByX, detectionCannyJoinByY,
        detectionBinaryThreshold));
  }

  storeFile.close();

  return true;
}

void ImageList::generate() {
  namespace fs = std::filesystem;
  
  store.clear();

  for (const auto &file : fs::directory_iterator(dirPath)) {
    if (std::string(file.path().filename())[0] == '.') {
      std::cout << "Skipping: " << file.path() << '\n';
      continue;
    }

    std::cout << "Reading: " << file.path();

    cv::Mat sourceImage = cv::imread(file.path());

    if (sourceImage.empty()) {
      std::cout << " (skipping, cannot read)\n";
      continue;
    } else {
      std::cout << '\n';
    }

    auto edgesMat = detectEdgesCanny(sourceImage);
    auto edges = edgesToBitset(edgesMat);
    store.push_back(std::make_shared<EdgedImage>(file.path(), sourceImage.cols,
                                                 sourceImage.rows, edges));
  }
}

void ImageList::save() {
  std::filesystem::path storePath { dirPath };
  storePath.append(".store");
  std::ofstream storeFile(storePath);
  if (!storeFile) {
    std::cerr << "Failed to open store file.\n";
    exit(1);
  }

  for (const std::shared_ptr<EdgedImage> &image : store) {
    storeFile << *image << '\n';
  }
}

void ImageList::sortBy(const ImageList::sort_predicate &sortFn) {
  std::sort(store.begin(), store.end(), sortFn);
}
void ImageList::sortBy(const char* sorter) {
  if (strcmp(sorter, "match-percentage") == 0) {
    sortBy([](std::shared_ptr<EdgedImage> a,
              std::shared_ptr<EdgedImage> b) -> bool {
      return a->lastMatch.percentage > b->lastMatch.percentage;
    });
  } else {
    throw std::runtime_error("Invalid sorter specified");
  }
}

// @todo is there a nicer way to do this?
ImageList::ImageStore::iterator ImageList::begin() {
  return store.begin();
}

ImageList::ImageStore::iterator ImageList::end() {
  return store.end();
}

ImageList::ImageStore::reference ImageList::at(size_t pos) {
  return store.at(pos);
}

int ImageList::count() const {
  return store.size();
}
