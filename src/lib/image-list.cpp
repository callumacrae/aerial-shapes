#include "image-list.hpp"

ImageList::ImageList() {};

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
    size_t first = line.find(',');
    size_t second = line.find(',', first + 1);
    size_t third = line.find(',', second + 1);

    std::string path = line.substr(0, first);
    int width = std::stoi(line.substr(first + 1, second - first - 1));
    int height = std::stoi(line.substr(second + 1, third - second - 1));
    boost::dynamic_bitset<unsigned char> edges(line.substr(third + 1));

    store.emplace_back(path, width, height, edges);
    count_++;
  }

  storeFile.close();

  return true;
}

void ImageList::generate() {
  namespace fs = std::filesystem;
  
  store.clear();
  count_ = 0;

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
    store.emplace_back(file.path(), sourceImage.cols, sourceImage.rows, edges);

    count_++;
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

  for (const EdgedImage &image : store) {
    storeFile << image << '\n';
  }
}

// @todo is there a nicer way to do this?
std::vector<EdgedImage>::iterator ImageList::begin() {
  return store.begin();
}

std::vector<EdgedImage>::iterator ImageList::end() {
  return store.end();
}

EdgedImage ImageList::at(size_t pos) {
  return store.at(pos);
}

int ImageList::count() const {
  return count_;
}
