#include "image-list.hpp"
#include "bitset-serialise.hpp"

ImageList::ImageList(std::string dirPath) : dirPath(dirPath) {
  namespace fs = std::filesystem;

  if (!fs::exists(dirPath)) {
    throw std::runtime_error("Directory doesn't exist: " + dirPath);
  }

  getStored();
}

bool ImageList::getStored() {
  namespace fs = std::filesystem;

  fs::path storePath{dirPath};
  storePath.append(".store");
  std::ifstream storeFile(storePath);
  if (!storeFile) {
    return false;
  }

  std::string line;

  while (std::getline(storeFile, line)) {
    std::string path;
    int width, height, bitsetSize;
    boost::dynamic_bitset<unsigned char> edges;

    int detectionMode, detectionBlurSize, detectionBlurSigmaX,
        detectionBlurSigmaY, detectionCannyThreshold1, detectionCannyThreshold2,
        detectionCannyJoinByX, detectionCannyJoinByY, detectionBinaryThreshold;

    std::stringstream lineStream(line);
    int i = 0;
    while (lineStream.good()) {
      std::string substr;
      std::getline(lineStream, substr, ',');

      if (i == 0) {
        path = substr;
      } else if (i == 1) {
        width = std::stoi(substr);
      } else if (i == 2) {
        height = std::stoi(substr);
      } else if (i == 3) {
        bitsetSize = std::stoi(substr);
      } else if (i == 4) {
        edges = stringToBitset(substr.c_str(), bitsetSize);
      } else if (i == 5) {
        detectionMode = std::stoi(substr);
      } else if (i == 6) {
        detectionBlurSize = std::stoi(substr);
      } else if (i == 7) {
        detectionBlurSigmaX = std::stoi(substr);
      } else if (i == 8) {
        detectionBlurSigmaY = std::stoi(substr);
      } else if (i == 9) {
        detectionCannyThreshold1 = std::stoi(substr);
      } else if (i == 10) {
        detectionCannyThreshold2 = std::stoi(substr);
      } else if (i == 11) {
        detectionBinaryThreshold = std::stoi(substr);
      } else if (i == 12) {
        detectionCannyJoinByX = std::stoi(substr);
      } else if (i == 13) {
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

    addFile(file);
  }
}

int ImageList::sync() {
  namespace fs = std::filesystem;

  int added = 0;

  for (const auto &file : fs::directory_iterator(dirPath)) {
    if (std::string(file.path().filename())[0] == '.') {
      continue;
    }

    bool exists = false;
    for (const std::shared_ptr<EdgedImage> &image : store) {
      if (image->path == std::string(file.path())) {
        exists = true;
        break;
      }
    }
    if (exists) {
      continue;
    }

    addFile(file);
    added++;
  }

  return added;
}

void ImageList::addFile(const std::filesystem::directory_entry &file) {
  std::cout << "Reading: " << file.path();

  cv::Mat sourceImage = cv::imread(file.path());

  if (sourceImage.empty()) {
    std::cout << " (skipping, cannot read)\n";
    return;
  } else {
    std::cout << '\n';
  }

  auto edgesMat = detectEdgesCanny(sourceImage);
  auto edges = edgesToBitset(edgesMat);
  store.push_back(std::make_shared<EdgedImage>(file.path(), sourceImage.cols,
                                               sourceImage.rows, edges));
}

// If saving before quitting, make sure you call async=false or you might
// destroy the store file
void ImageList::save(bool async) {
  auto threadFn = [&]() {
    std::filesystem::path storePath{dirPath};
    storePath.append(".store");
    std::ofstream storeFile(storePath);
    if (!storeFile) {
      throw std::runtime_error("Failed to open store file.");
    }

    for (const std::shared_ptr<EdgedImage> &image : store) {
      storeFile << *image << '\n';
    }
  };

  if (async) {
    std::thread saveThread(threadFn);
    saveThread.detach();
  } else {
    threadFn();
  }
}

int ImageList::matchTo(const cv::Mat &templateImage, ImageMatch *bestMatch,
                       EdgedImage **bestMatchImage, float offsetScaleStep,
                       int offsetXStep, int offsetYStep, float minOffsetScale,
                       int maxOffset, float whiteBias) {
  std::atomic_int runs(0);
  int maxThreads = std::thread::hardware_concurrency() - 1;
  if (maxThreads == -1) {
    throw std::runtime_error("hardware_concurrency() returning 0, unsupported");
  }
  int threads = std::min(maxThreads, count());

  std::vector<std::thread> threadVector;
  threadVector.reserve(threads);

  std::atomic_int imageIndex(0);
  std::mutex bestMatchMutex;

  auto threadFn = [&]() {
    while (true) {
      int indexToGet = imageIndex++;
      if (indexToGet > store.size() - 1) {
        return;
      }
      std::shared_ptr<EdgedImage> sourceImage = store[indexToGet];

      ImageMatch match;
      runs += sourceImage->matchTo(templateImage, &match, offsetScaleStep,
                                   offsetXStep, offsetYStep, minOffsetScale,
                                   maxOffset, whiteBias);

      std::lock_guard<std::mutex> bestMatchLock(bestMatchMutex);

      if (match.percentage > bestMatch->percentage) {
        *bestMatch = match;
        *bestMatchImage = sourceImage.get();
      }
    }
  };

  for (int i = 0; i < threads; ++i) {
    threadVector.emplace_back(threadFn);
  }
  for (std::thread &t : threadVector) {
    if (t.joinable()) {
      t.join();
    }
  }

  return runs;
};

void ImageList::sortBy(const ImageList::sort_predicate &sortFn) {
  std::sort(store.begin(), store.end(), sortFn);
}
void ImageList::sortBy(const char *sorter) {
  if (strcmp(sorter, "match-percentage") == 0) {
    sortBy([](std::shared_ptr<EdgedImage> a,
              std::shared_ptr<EdgedImage> b) -> bool {
      return a->lastMatch.percentage > b->lastMatch.percentage;
    });
  } else if (strcmp(sorter, "path") == 0) {
    sortBy([](std::shared_ptr<EdgedImage> a,
              std::shared_ptr<EdgedImage> b) -> bool {
      return a->path < b->path;
    });
  } else {
    throw std::runtime_error("Invalid sorter specified");
  }
}

// @todo is there a nicer way to do this?
ImageList::image_store::iterator ImageList::begin() { return store.begin(); }

ImageList::image_store::iterator ImageList::end() { return store.end(); }

ImageList::image_store::reference ImageList::at(size_t pos) {
  return store.at(pos);
}

void ImageList::erase(size_t pos) {
  store.erase(begin() + pos);
}

int ImageList::count() const { return store.size(); }
