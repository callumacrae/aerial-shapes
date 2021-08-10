#include "edged-image.hpp"

// these vars can probs be hardcoded once i've figured out what it should be
int EdgedImage::matchTo(const cv::Mat &templateImage, ImageMatch *match,
                        float offsetScaleStep, int offsetXStep, int offsetYStep,
                        float minOffsetScale, int maxOffset, float whiteBias) {
  int channels = templateImage.channels();
  CV_Assert(channels == 1);

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

            ImageMatch match;
            matchToStep(templateImage, edgesAry, &match, scale,
                        originX + offsetX, originY + offsetY, 10, whiteBias);

            if (runs == 0 || (match.percentage > 0.5 &&
                              match.percentage > bestMatch.percentage - 0.1)) {
              matchToStep(templateImage, edgesAry, &match, scale,
                          originX + offsetX, originY + offsetY, 1, whiteBias);
              fullRuns++;

              if (match.percentage > bestMatch.percentage) {
                bestMatch = match;
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
                             float whiteBias) const {
  int testedBlack = 0;
  int matchingBlack = 0;
  int testedWhite = 0;
  int matchingWhite = 0;

  for (int y = 0; y < templateImage.rows; y += rowStep) {
    const uchar *p = templateImage.ptr<uchar>(y);

    for (int x = 0; x < templateImage.cols; ++x) {
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
