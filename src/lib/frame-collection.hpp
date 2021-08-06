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
  std::vector<MatchData> _cachedMatches;
  std::vector<cv::Mat> _cachedImages;

public:
  FrameCollection() {};
  FrameCollection(std::string &name);
  void addFrame(ImageList imageList);
  void popFrame();
  void save(std::string &name);

  MatchData matchAt(int pos);
  cv::Mat imageAt(int pos);
  void preloadAll();

  friend std::ostream& operator<<(std::ostream& os, const FrameCollection& frames);
};

std::ostream& operator<<(std::ostream& os, const MatchData& matchData);
std::ostream& operator<<(std::ostream& os, const FrameData& frameData);
std::ostream& operator<<(std::ostream& os, const FrameCollection& frames);
