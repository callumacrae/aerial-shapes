#include "frame-collection.hpp"

FrameCollection::FrameCollection(const std::string &name) {
  namespace fs = std::filesystem;

  std::filesystem::path storePath("assets/collections");
  storePath.append(name);
  storePath.append(".frame-data");
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
  _isCachedMatches.resize(size());
  std::fill(_isCachedMatches.begin(), _isCachedMatches.end(), false);

  _cachedImages.resize(size());
  _isCachedImages.resize(size());
  std::fill(_isCachedImages.begin(), _isCachedImages.end(), false);
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

void FrameCollection::popFrame() { pop_back(); }

void FrameCollection::save(const std::string &name) {
  namespace fs = std::filesystem;

  if (empty() || name.empty()) {
    // todo do something less damaging
    throw std::runtime_error("Invalid save");
  }

  fs::path storePath("assets/collections");
  storePath.append(name);

  fs::create_directory(storePath);

  storePath.append(".frame-data");

  std::ofstream storeFile(storePath);
  if (!storeFile) {
    throw std::runtime_error("Failed to open store file.");
  }

  storeFile << *this;
}

// Currently this function uses greedy matching to grab the first match
// containing an image that hasn't already been used. In the future it would
// be great if it implemented the hungarian algorithm for better matching.
std::vector<MatchData>::iterator FrameCollection::matchAt(int pos) {
  if (_isCachedMatches.at(pos)) {
    return _cachedMatches.at(pos);
  }

  std::vector<MatchData>::iterator proposedMatch = at(pos).frames.begin();
  for (; proposedMatch < at(pos).frames.end(); ++proposedMatch) {
    bool alreadyUsed = false;
    for (int i = 0; i < pos; ++i) {
      if (matchAt(i)->path == proposedMatch->path) {
        alreadyUsed = true;
        break;
      }
    }

    if (!alreadyUsed) {
      break;
    }
  }

  _cachedMatches.at(pos) = proposedMatch;
  _isCachedMatches.at(pos) = true;

  return proposedMatch;
}

std::vector<MatchData>* FrameCollection::matchesAt(int pos) {
  return &at(pos).frames;
}

cv::Mat FrameCollection::imageAt(int pos) {
  if (_isCachedImages.at(pos)) {
    return _cachedImages.at(pos);
  }

  std::vector<MatchData>::iterator match = matchAt(pos);
  cv::Mat image = imageFor(match);

  _cachedImages.at(pos) = image;
  _isCachedImages.at(pos) = true;

  return image;
}

// todo move caching to this?
cv::Mat FrameCollection::imageFor(std::vector<MatchData>::iterator match) {
  cv::Mat image = cv::imread(match->path);

  if (image.empty()) {
    throw std::runtime_error("Couldn't read source image");
  }

  float realScale = (float)image.cols / STORED_EDGES_WIDTH;

  cv::Rect roi;
  roi.x = round(match->originX * realScale);
  roi.y = round(match->originY * realScale);
  roi.width = round(CANVAS_WIDTH * realScale * match->scale);
  roi.height = round(CANVAS_HEIGHT * realScale * match->scale);

  cv::Mat cropped = image(roi);
  cv::Mat scaledImage;
  cv::resize(cropped, scaledImage, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

  return scaledImage;
}

void FrameCollection::forceMatch(int pos, std::vector<MatchData>::iterator match) {
  MatchData matchCopy = *match;
  at(pos).frames.clear();
  at(pos).frames.push_back(matchCopy);

  std::fill(_isCachedMatches.begin(), _isCachedMatches.end(), false);
  std::fill(_isCachedImages.begin(), _isCachedImages.end(), false);

  match = at(pos).frames.begin();

  // Delete this image from all other frames
  FrameData* current = &at(pos);
  for (FrameData &otherFramesData : *this) {
    if (&otherFramesData == current) {
      continue;
    }

    std::vector<MatchData> &otherFrames = otherFramesData.frames;

    auto otherPos = std::find_if(otherFrames.begin(), otherFrames.end(),
        [&match](const MatchData &otherMatch) {
      return match->path == otherMatch.path;
    });

    otherFrames.erase(otherPos);
  }
}

void FrameCollection::removeMatch(int pos, std::vector<MatchData>::iterator match) {
  at(pos).frames.erase(match);
}

void FrameCollection::preloadAll() {
  for (int i = 0; i < size(); ++i) {
    imageAt(i);
  }
}

void FrameCollection::writeImages(const std::string &name) {
  for (size_t i = 0; i < size(); ++i) {
    cv::Mat image = imageAt(i);

    char frameName[8];
    sprintf(frameName, "%03d.jpg", (int)i);
    std::filesystem::path imagePath("assets/collections");
    imagePath.append(name);
    imagePath.append(frameName);

    cv::imwrite(imagePath.string(), image);
  }
}

std::ostream &operator<<(std::ostream &os, const MatchData &matchData) {
  os << matchData.path << ',' << matchData.percentage << ',' << matchData.scale
     << ',' << matchData.originX << ',' << matchData.originY;
  return os;
}

std::ostream &operator<<(std::ostream &os, const FrameData &frameData) {
  for (int i = 0; i < frameData.frames.size(); ++i) {
    if (i != 0) {
      os << ',';
    }
    os << '[' << frameData.frames[i] << ']';
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const FrameCollection &frames) {
  for (const FrameData &frameData : frames) {
    os << frameData << '\n';
  }
  return os;
}
