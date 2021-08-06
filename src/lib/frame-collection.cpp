#include "frame-collection.hpp"

FrameCollection::FrameCollection(std::string &name) {
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

      std::stringstream matchStream(matchDataStr);
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

    push_back(std::move(frameData));
  }

  _cachedMatches.resize(size());
  _cachedImages.resize(size());
}

void FrameCollection::addFrame(ImageList imageList) {
  FrameData frameData;

  imageList.sortBy("match-percentage");

  for (std::shared_ptr<EdgedImage> &image : imageList) {
    frameData.frames.emplace_back(
        image->path, image->lastMatch.percentage, image->lastMatch.scale,
        image->lastMatch.originX, image->lastMatch.originY);
  }

  push_back(std::move(frameData));
}

void FrameCollection::popFrame() {
  pop_back();
}

void FrameCollection::save(std::string &name) {
  if (empty() || name.empty()) {
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

// Currently this function uses greedy matching to grab the first match
// containing an image that hasn't already been used. In the future it would
// be great if it implemented the hungarian algorithm for better matching.
MatchData FrameCollection::matchAt(int pos) {
  MatchData cachedMatch = _cachedMatches.at(pos);
  if (!cachedMatch.path.empty()) {
    return cachedMatch;
  }

  std::vector<MatchData>::iterator proposedMatch = at(pos).frames.begin();
  for (; proposedMatch < at(pos).frames.end(); ++proposedMatch) {
    bool alreadyUsed = false;
    for (int i = 0; i < pos; ++i) {
      if (matchAt(i).path == proposedMatch->path) {
        alreadyUsed = true;
        break;
      }
    }

    if (!alreadyUsed) {
      break;
    }
  }

  _cachedMatches.at(pos) = *proposedMatch;

  return *proposedMatch;
}

cv::Mat FrameCollection::imageAt(int pos) {
  cv::Mat cachedImage = _cachedImages.at(pos);
  if (!cachedImage.empty()) {
    return cachedImage;
  }

  MatchData match = matchAt(pos);
  cv::Mat image = cv::imread(match.path);

  if (image.empty()) {
    throw std::runtime_error("Couldn't read source image");
  }

  float realScale = (float)image.cols / STORED_EDGES_WIDTH;

  cv::Rect roi;
  roi.x = round(match.originX * realScale);
  roi.y = round(match.originY * realScale);
  roi.width = round(CANVAS_WIDTH * realScale * match.scale);
  roi.height = round(CANVAS_HEIGHT * realScale * match.scale);

  cv::Mat cropped = image(roi);
  cv::Mat scaledImage;
  cv::resize(cropped, scaledImage, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

  _cachedImages.at(pos) = scaledImage;

  return scaledImage;
}

void FrameCollection::preloadAll() {
  for (int i = 0; i < size(); ++i) {
    imageAt(i);
  }
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
  for (const FrameData &frameData : frames) {
    os << frameData << '\n';
  }
  return os;
}
