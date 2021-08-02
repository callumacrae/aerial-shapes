#include "../config.h"

#include "edit-image-edges.hpp"

std::optional<EdgedImage*> editImageEdges(EdgedImage &image) {
  int detectionMode = image.detectionMode;
  int blurSize = image.detectionBlurSize;
  int sigmaX = image.detectionBlurSigmaX;
  int sigmaY = image.detectionBlurSigmaY;
  int threshold1 = image.detectionCannyThreshold1;
  int threshold2 = image.detectionCannyThreshold2;
  int joinByX = image.detectionCannyJoinByX;
  int joinByY = image.detectionCannyJoinByY;
  int binaryThreshold = image.detectionBinaryThreshold;

  const cv::Mat sourceImage = cv::imread(image.path);
  cv::Mat templateImage = image.edgesAsMatrix();

  if (sourceImage.empty()) {
    throw std::runtime_error("Could not read source image");
  }

  std::string title = std::string("Editing ") + image.path;
  initWindow(sourceImage.cols, sourceImage.rows, title.c_str());
  GLuint image_tex;

  auto generatePreviewTexture = [&]() {
    // todo don't run on initial run
    if (detectionMode == ImageEdgeMode_Canny) {
      templateImage = detectEdgesCanny(sourceImage, blurSize, sigmaX, sigmaY,
                                       threshold1, threshold2, joinByX,
                                       joinByY);
    } else if (detectionMode == ImageEdgeMode_Threshold) {
      templateImage = detectEdgesThreshold(sourceImage, blurSize, sigmaX,
                                           sigmaY, binaryThreshold);
    } else {
      throw std::invalid_argument("Manual edge editing isn't supported yet");
    }

    cv::Mat edges, mask, scaledEdges, scaledPlusEdges;
    cv::cvtColor(templateImage, edges, cv::COLOR_GRAY2BGR);

    cv::threshold(templateImage, mask, 0, 255, cv::THRESH_BINARY);
    edges.setTo(cv::Scalar(255, 0, 0), mask);

    cv::resize(edges, scaledEdges, sourceImage.size(), 0, 0, cv::INTER_NEAREST);
    cv::bitwise_or(scaledEdges, sourceImage, scaledPlusEdges);

    matToTexture(scaledPlusEdges, &image_tex);
  };

  generatePreviewTexture();

  bool save = false;

  openWindow([&](GLFWwindow* window) {
    bool changed = false;

    ImGui::SetNextWindowFocus();
    /* ImGui::ShowDemoWindow(); */
    ImGui::Begin("Controls");

    ImGui::Text("Detection Mode:");
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Canny", &detectionMode, ImageEdgeMode_Canny);
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Threshold", &detectionMode, ImageEdgeMode_Threshold);
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Manual", &detectionMode, ImageEdgeMode_Manual);

    ImGui::NewLine();

    changed |= ImGui::SliderInt("Blur size", &blurSize, 0, 50);
    changed |= ImGui::SliderInt("Blur sigma X", &sigmaX, 0, 20);
    changed |= ImGui::SliderInt("Blur sigma Y", &sigmaY, 0, 20);

    ImGui::NewLine();

    if (detectionMode == ImageEdgeMode_Canny) {
      changed |= ImGui::SliderInt("Canny threshold 1", &threshold1, 0, 255);
      changed |= ImGui::SliderInt("Canny threshold 2", &threshold2, 0, 255);
      changed |= ImGui::SliderInt("Join by x", &joinByX, 0, 39);
      changed |= ImGui::SliderInt("Join by y", &joinByY, 0, 39);
    } else if (detectionMode == ImageEdgeMode_Threshold) {
      changed |= ImGui::SliderInt("Binary threshold", &binaryThreshold, 0, 255);
    } else {
      ImGui::Text("Filler text for manual mode");
    }

    ImGui::NewLine();

    if (ImGui::Button("Save")) {
      save = true;
      return true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard")) {
      return true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      detectionMode = ImageEdgeMode_Canny;
      blurSize = EDGE_DETECTION_BLUR_SIZE;
      sigmaX = EDGE_DETECTION_BLUR_SIGMA_X;
      sigmaY = EDGE_DETECTION_BLUR_SIGMA_Y;
      threshold1 = EDGE_DETECTION_CANNY_THRESHOLD_1;
      threshold2 = EDGE_DETECTION_CANNY_THRESHOLD_2;
      joinByX = EDGE_DETECTION_CANNY_JOIN_BY_X;
      joinByY = EDGE_DETECTION_CANNY_JOIN_BY_Y;
      binaryThreshold = EDGE_DETECTION_BINARY_THRESHOLD;
      changed = true;
    }
    ImGui::End();

    if (changed) {
      if (blurSize % 2 == 0) {
        blurSize++;
      }

      if (joinByX != 0 && joinByX % 2 == 0) {
        joinByX++;
      }

      if (joinByY != 0 && joinByY % 2 == 0) {
        joinByY++;
      }

      generatePreviewTexture();
    }

    // Image window is full screen and non-interactive - basically, a background
    int actualWidth, actualHeight;
    glfwGetWindowSize(window, &actualWidth, &actualHeight);
    ImGui::SetNextWindowPos(ImVec2(.0f, .0f));
    ImGui::SetNextWindowSize(ImVec2(actualWidth, actualHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("Image", NULL, staticWindowFlags);
    ImGui::Image((void *)(intptr_t)image_tex, ImGui::GetContentRegionMax());
    ImGui::End();
    ImGui::PopStyleVar(1);

    return false;
  });

  glDeleteTextures(1, &image_tex);

  if (!save) {
    return std::nullopt;
  }

  auto edges = edgesToBitset(templateImage);
  return new EdgedImage(image.path, image.width, image.height, edges,
                        detectionMode, blurSize, sigmaX, sigmaY, threshold1,
                        threshold2, joinByX, joinByY, binaryThreshold);
}

