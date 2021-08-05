#include "frame-collection.hpp"

FrameCollection::FrameCollection(std::string &name) {
  std::cout << "Loading " << name << '\n';
  namespace fs = std::filesystem;

  std::filesystem::path storePath("assets/collections");
  storePath.append(name);
  std::ifstream storeFile(storePath);

  if (!storeFile) {
    throw std::runtime_error("Failed to open store file.");
  }

  std::string line;
  while (std::getline(storeFile, line)) {
    FrameData frameData;

    std::stringstream lineStream(line);
    while (lineStream.good()) {
      std::string matchDataStrRaw;
      std::getline(lineStream, matchDataStrRaw, ']');

      if (matchDataStrRaw.find('[') == std::string::npos) {
        break;
      }

      std::string matchDataStr =
          matchDataStrRaw.substr(matchDataStrRaw.find('[') + 1);
      MatchData matchData;

      std::stringstream(matchStream);
      int i = 0;
      while (matchStream.good()) {
        std::string substr;
        std::getline(matchStream, substr, ',');

        if (i == 0) {
          matchData.path = substr;
        } else if (i == 1) {
          matchData.percentage = std::stof(substr);
        } else if (i == 2) {
          matchData.scale = std::stof(substr);
        } else if (i == 3) {
          matchData.originX = std::stoi(substr);
        } else if (i == 4) {
          matchData.originY = std::stoi(substr);
        }

        i++;
      }

      frameData.frames.push_back(std::move(matchData));
    }

    _collection.push_back(std::move(frameData));
  }
}

void FrameCollection::addFrame(ImageList imageList) {
  FrameData frameData;

  imageList.sortBy("match-percentage");

  for (std::shared_ptr<EdgedImage> &image : imageList) {
    frameData.frames.emplace_back(
        image->path, image->lastMatch.percentage, image->lastMatch.scale,
        image->lastMatch.originX, image->lastMatch.originY);
  }

  _collection.push_back(std::move(frameData));
}

void FrameCollection::popFrame() {
  _collection.pop_back();
}

void FrameCollection::save(std::string &name) {
  if (_collection.empty() || name.empty()) {
    // todo do something less damaging
    throw std::runtime_error("Invalid save");
  }

  std::filesystem::path storePath("assets/collections");
  storePath.append(name);
  std::ofstream storeFile(storePath);
  if (!storeFile) {
    throw std::runtime_error("Failed to open store file.");
  }

  storeFile << *this;
}

int FrameCollection::size() const {
  return _collection.size();
}

std::ostream &operator<<(std::ostream &os, const MatchData& matchData) {
  os << matchData.path << ',' << matchData.percentage << ',' << matchData.scale
    << ',' << matchData.originX << ',' << matchData.originY;
  return os;
}

std::ostream &operator<<(std::ostream &os, const FrameData& frameData) {
  for (int i = 0; i < frameData.frames.size(); ++i) {
    if (i != 0) {
      os << ',';
    }
    os << '[' << frameData.frames[i] << ']';
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const FrameCollection& frames) {
  for (const FrameData &frameData : frames._collection) {
    os << frameData << '\n';
  }
  return os;
}
