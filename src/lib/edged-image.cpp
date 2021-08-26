#include "edged-image.hpp"

void EdgedImage::provideMatchContext(int templateOffsetX, int templateOffsetY) {
  _matchContextOffsetX = templateOffsetX;
  _matchContextOffsetY = templateOffsetY;
}
void EdgedImage::resetMatchContext() {
  _matchContextOffsetX = 0;
  _matchContextOffsetY = 0;
}

int EdgedImage::matchTo(const cv::Mat &templateImageIn, ImageMatch *match,
                        float offsetScaleStep, int offsetXStep, int offsetYStep,
                        float minOffsetScale, int maxOffset, float whiteBias) {
  int channels = templateImageIn.channels();
  CV_Assert(channels == 1);

  cv::Mat templateImage;
  if (_matchContextOffsetX || _matchContextOffsetY) {
    cv::Mat normalisedTemplateImage =
        cv::Mat::zeros(templateImageIn.size(), templateImageIn.type());
    int x1 = _matchContextOffsetX < 0 ? 0 : _matchContextOffsetX;
    int rectWidth = templateImageIn.cols - std::abs(_matchContextOffsetX);
    int y1 = _matchContextOffsetY < 0 ? 0 : _matchContextOffsetY;
    int rectHeight = templateImageIn.rows - std::abs(_matchContextOffsetY);
    templateImageIn(cv::Rect(x1, y1, rectWidth, rectHeight))
        .copyTo(normalisedTemplateImage(cv::Rect(x1 - _matchContextOffsetX,
                                                 y1 - _matchContextOffsetY,
                                                 rectWidth, rectHeight)));

    // todo fix these copies, especially the second one
    templateImage = normalisedTemplateImage;
  } else {
    templateImage = templateImageIn;
  }

  // Converting to an array means we only have to do the bitwise operations
  // required to access a bitset once per run
  uchar edgesAry[edges.size()];
  for (int i = 0; i < edges.size(); ++i) {
    edgesAry[i] = edges[i] ? 255 : 0;
  }

  int sourceImageActualHeight = (float)STORED_EDGES_WIDTH / width * height;
  float scaleX = (float)STORED_EDGES_WIDTH / templateImage.cols;
  float scaleY = (float)sourceImageActualHeight / templateImage.rows;

  float scaleBase = fmin(scaleX, scaleY);

  ImageMatch bestMatch;

  minOffsetScale = fmax(minOffsetScale, (float)OUTPUT_WIDTH / width);

  int runs = 0;
  int fullRuns = 0;

  for (float offsetScale = 1; offsetScale >= minOffsetScale;
       offsetScale -= offsetScaleStep) {
    float scale = scaleBase * offsetScale;

    int originX = 0;
    int originY = 0;

    if (scaleX != 0) {
      originX = (STORED_EDGES_WIDTH - templateImage.cols * scale) / 2;
    }
    if (scaleX != 0) {
      originY = (sourceImageActualHeight - templateImage.rows * scale) / 2;
    }

    // todo vary step and max depending on scale?
    int maxOffsetX = std::min(maxOffset, originX);
    int maxOffsetY = std::min(maxOffset, originY);

    for (int offsetXRoot = 0; offsetXRoot <= maxOffsetX;
         offsetXRoot += offsetXStep) {
      for (int offsetXMultiplier = -1; offsetXMultiplier <= 1;
           offsetXMultiplier += 2) {
        // Search inside out, e.g. 0 -1 1 -2 2 -3 3
        int offsetX = offsetXRoot * offsetXMultiplier;

        for (int offsetYRoot = 0; offsetYRoot <= maxOffsetY;
             offsetYRoot += offsetYStep) {
          for (int offsetYMultiplier = -1; offsetYMultiplier <= 1;
               offsetYMultiplier += 2) {
            int offsetY = offsetYRoot * offsetYMultiplier;

            // Calculate if template offset is viable
            {
              float realScale = (float)width / STORED_EDGES_WIDTH;
              int finalX = originX + offsetX - _matchContextOffsetX * scale;
              int finalY = originY + offsetY - _matchContextOffsetY * scale;
              cv::Rect roi;
              roi.x = round(finalX * realScale);
              roi.y = round(finalY * realScale);
              roi.width = round(CANVAS_WIDTH * realScale * scale);
              roi.height = round(CANVAS_HEIGHT * realScale * scale);

              if (roi.x < 0 || roi.x + roi.width > width) {
                // We're in the y loop so we can break here - will be invalid
                // for every item in the loop
                break;
              }
              if (roi.y < 0 || roi.y + roi.height > height) {
                continue;
              }
            }

            ImageMatch match;
            if (runs != 0) {
              matchToStep(templateImage, edgesAry, &match, scale,
                          originX + offsetX, originY + offsetY, 10, 1,
                          whiteBias);

              // If partial match on rows isn't good enough, run again on cols
              if (match.percentage < 0.5 ||
                  match.percentage < bestMatch.percentage - 0.1) {
                matchToStep(templateImage, edgesAry, &match, scale,
                            originX + offsetX, originY + offsetY, 1, 10,
                            whiteBias);
              }
            }

            if (runs == 0 || (match.percentage > 0.5 &&
                              match.percentage > bestMatch.percentage - 0.1)) {
              matchToStep(templateImage, edgesAry, &match, scale,
                          originX + offsetX, originY + offsetY, 1, 1, whiteBias);
              fullRuns++;

              if (match.percentage > bestMatch.percentage) {
                bestMatch = match;
                bestMatch.originX -= _matchContextOffsetX * scale;
                bestMatch.originY -= _matchContextOffsetY * scale;
              }
            }

            runs++;

            if (offsetY == 0) {
              break;
            }
          }
        }

        // No need to multiply 0 by both -1 and 1
        if (offsetX == 0) {
          break;
        }
      }
    }
  }

  *match = bestMatch;
  lastMatch = bestMatch; // Have to copy here or can cause segfaults
  return runs;
}

void EdgedImage::matchToStep(const cv::Mat &templateImage,
                             const uchar edgesAry[], ImageMatch *match,
                             float scale, int originX, int originY, int rowStep,
                             int colStep, float whiteBias) const {
  int testedBlack = 0;
  int matchingBlack = 0;
  int testedWhite = 0;
  int matchingWhite = 0;

  for (int y = 0; y < templateImage.rows; y += rowStep) {
    const uchar *p = templateImage.ptr<uchar>(y);

    for (int x = 0; x < templateImage.cols; x += colStep) {
      bool templatePixVal = p[x] != 0;

      int transformedX = originX + floor((float)x * scale);
      int transformedY = originY + floor((float)y * scale);
      int i = transformedY * STORED_EDGES_WIDTH + transformedX;
      bool sourcePixVal = edgesAry[i];

      if (templatePixVal == 1) {
        if (templatePixVal == sourcePixVal) {
          ++matchingWhite;
        }
        ++testedWhite;
      } else {
        if (templatePixVal == sourcePixVal) {
          ++matchingBlack;
        }
        ++testedBlack;
      }
    }
  }

  float percentageBlack = (float)matchingBlack / testedBlack;
  float percentageWhite = (float)matchingWhite / testedWhite;
  float percentage =
      percentageWhite * whiteBias + percentageBlack * (1 - whiteBias);
  *match = ImageMatch{percentage, scale, originX, originY};
}

cv::Mat EdgedImage::edgesAsMatrix() const {
  uchar edgesAry[edges.size()];
  for (int i = 0; i < edges.size(); ++i) {
    edgesAry[i] = edges[i] ? 255 : 0;
  }

  int cols = STORED_EDGES_WIDTH;
  int rows = edges.size() / cols;

  cv::Mat mat(rows, cols, CV_8UC1);
  memcpy(mat.data, edgesAry, edges.size() * sizeof(uchar));

  return mat;
}

// Cache in memory - takes surprisingly long to read from disk every time
cv::Mat EdgedImage::getOriginal(bool cache) {
  if (!cache) {
    return cv::imread(path);
  }

  if (!originalImage.empty()) {
    return originalImage;
  }

  originalImage = cv::imread(path);
  return originalImage;
}

std::ostream &operator<<(std::ostream &os, const EdgedImage &image) {
  os << image.path << ',' << image.width << ',' << image.height << ','
     << image.edges.size() << ',' << bitsetToString(image.edges) << ','
     << image.detectionMode << ',' << image.detectionBlurSize << ','
     << image.detectionBlurSigmaX << ',' << image.detectionBlurSigmaY << ','
     << image.detectionCannyThreshold1 << ',' << image.detectionCannyThreshold2
     << ',' << image.detectionBinaryThreshold << ','
     << image.detectionCannyJoinByX << ',' << image.detectionCannyJoinByY;
  return os;
}
