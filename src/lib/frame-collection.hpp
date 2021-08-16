#pragma once

#include <filesystem>
#include <fstream>

#include "../precompiled.h"

#include "image-list.hpp"

struct MatchData {
  std::string path;
  float percentage, scale;
  int originX, originY;

  MatchData() {}
  MatchData(std::string &path, float percentage, float scale, int originX,
            int originY)
      : path(path), percentage(percentage), scale(scale), originX(originX),
        originY(originY) {}
};

// todo refactor this
struct FrameData {
  std::vector<MatchData> frames;
};

class FrameCollection : public std::vector<FrameData> {
  std::vector<bool> _isCachedMatches;
  std::vector<std::vector<MatchData>::iterator> _cachedMatches;
  std::vector<bool> _isCachedImages;
  std::vector<cv::Mat> _cachedImages;

  std::vector<MatchData>::iterator _uncached;

  void purgeCache();

public:
  FrameCollection() {};
  FrameCollection(const std::string &name);
  void addFrame(ImageList imageList);
  void popFrame();
  void save(const std::string &name);

  std::vector<MatchData>::iterator matchAt(int pos);
  std::vector<MatchData>* matchesAt(int pos);
  cv::Mat imageAt(int pos);
  cv::Mat imageFor(std::vector<MatchData>::iterator match);
  void forceMatch(int pos, std::vector<MatchData>::iterator match);
  void removeMatch(int pos, std::vector<MatchData>::iterator match);
  void editMatchScale(int pos, float newScale);
  void editMatchOriginX(int pos, int originX);
  void editMatchOriginY(int pos, int originY);

  void preloadAll();
  void writeImages(const std::string &name);

  friend std::ostream& operator<<(std::ostream& os, const FrameCollection& frames);
};

std::ostream& operator<<(std::ostream& os, const MatchData& matchData);
std::ostream& operator<<(std::ostream& os, const FrameData& frameData);
std::ostream& operator<<(std::ostream& os, const FrameCollection& frames);
