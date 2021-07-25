#include "image-list.hpp"

ImageList::ImageList() {};

ImageList::ImageList(std::string dirPath, bool refreshCache) : dirPath(dirPath) {
  namespace fs = std::filesystem;

  if (!fs::exists(dirPath)) {
    // @todo probably shouldn't log + exit from here
    std::cerr << "Directory doesn't exist: " << dirPath << '\n';
    exit(1);
  }

  if (!refreshCache && getCached()) {
    fromCache_ = true;
  } else {
    generate();
  }
};

bool ImageList::getCached() {
  namespace fs = std::filesystem;

  fs::path cachePath { dirPath };
  cachePath.append(".cache");
  std::ifstream cacheFile(cachePath);
  if (!cacheFile) {
    return false;
  }

  std::string line;

  while (std::getline(cacheFile, line)) {
    size_t first = line.find(',');
    size_t second = line.find(',', first + 1);
    size_t third = line.find(',', second + 1);

    std::string path = line.substr(0, first);
    int width = std::stoi(line.substr(first + 1, second - first - 1));
    int height = std::stoi(line.substr(second + 1, third - second - 1));
    boost::dynamic_bitset<unsigned char> edges(line.substr(third + 1));

    store.emplace_front(path, width, height, edges);
    count_++;
  }

  cacheFile.close();

  return true;
}

void ImageList::generate() {
  namespace fs = std::filesystem;

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

    auto edges = detectEdgesAsBitset(sourceImage);
    store.emplace_front(
        file.path(), sourceImage.cols, sourceImage.rows, edges);

    count_++;
  }

  std::filesystem::path cachePath { dirPath };
  cachePath.append(".cache");
  std::ofstream cacheFile(cachePath);
  if (!cacheFile) {
    std::cerr << "Failed to open cache file.\n";
    exit(1);
  }

  for (const EdgedImage &image : store) {
    cacheFile << image << '\n';
  }
}

// @todo is there a nicer way to do this?
std::forward_list<EdgedImage>::iterator ImageList::begin() {
  return store.begin();
}

std::forward_list<EdgedImage>::iterator ImageList::end() {
  return store.end();
}

int ImageList::count() const {
  return count_;
}

bool ImageList::fromCache() const {
  return fromCache_;
}
