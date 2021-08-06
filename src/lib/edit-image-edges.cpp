#include "../config.h"

#include "edit-image-edges.hpp"

enum ManualEditModes {
  ManualEditMode_Erase,
  ManualEditMode_Line,
  ManualEditMode_Square,
  ManualEditMode_Circle
};

std::optional<EdgedImage *> editImageEdges(EdgedImage &image) {
  int detectionMode = image.detectionMode;
  int blurSize = image.detectionBlurSize;
  int sigmaX = image.detectionBlurSigmaX;
  int sigmaY = image.detectionBlurSigmaY;
  int threshold1 = image.detectionCannyThreshold1;
  int threshold2 = image.detectionCannyThreshold2;
  int joinByX = image.detectionCannyJoinByX;
  int joinByY = image.detectionCannyJoinByY;
  int binaryThreshold = image.detectionBinaryThreshold;
  int manualEditMode = ManualEditMode_Line;

  const cv::Mat sourceImage = cv::imread(image.path);
  cv::Mat templateImage = image.edgesAsMatrix();

  if (sourceImage.empty()) {
    throw std::runtime_error("Could not read source image");
  }

  std::string title = std::string("Editing ") + image.path;
  initWindow(sourceImage.cols, sourceImage.rows, title.c_str());
  GLuint image_tex;

  ImVec2 mouseDownPos(-1, -1);
  ImVec2 mouseCurrentPos(-1, -1);

  auto drawTo = [&](cv::Mat &edges, bool preview = false) {
    int storedEdgesHeight = (float)STORED_EDGES_WIDTH / edges.cols * edges.rows;

    cv::Point pointA(mouseDownPos.x * STORED_EDGES_WIDTH,
                     mouseDownPos.y * storedEdgesHeight);
    cv::Point pointB(mouseCurrentPos.x * STORED_EDGES_WIDTH,
                     mouseCurrentPos.y * storedEdgesHeight);

    cv::Scalar color =
        edges.channels() == 3 ? cv::Scalar(0, 0, 255) : cv::Scalar(255);

    if (manualEditMode == ManualEditMode_Erase) {
      cv::rectangle(edges, pointA, pointB, cv::Scalar(0), cv::FILLED);
      if (preview) {
        cv::rectangle(edges, pointA, pointB, color);
      }
    } else if (manualEditMode == ManualEditMode_Square) {
      cv::rectangle(edges, pointA, pointB, color);
    } else if (manualEditMode == ManualEditMode_Line) {
      cv::line(edges, pointA, pointB, color);
    } else if (manualEditMode == ManualEditMode_Circle) {
      cv::Point center = (pointA + pointB) / 2;
      int radius =
          sqrt(pow(pointA.x - pointB.x, 2) + pow(pointA.y - pointB.y, 2)) / 2;
      cv::circle(edges, center, radius, color);
    }
  };

  auto generatePreviewTexture = [&](bool initial) {
    // previewTemplateImage is for previewing manual edits
    cv::Mat previewTemplateImage;
    bool drawing =
        detectionMode == ImageEdgeMode_Manual && mouseDownPos.x != -1;

    if (!initial) {
      if (detectionMode == ImageEdgeMode_Canny) {
        templateImage =
            detectEdgesCanny(sourceImage, blurSize, sigmaX, sigmaY, threshold1,
                             threshold2, joinByX, joinByY);
      } else if (detectionMode == ImageEdgeMode_Threshold) {
        templateImage = detectEdgesThreshold(sourceImage, blurSize, sigmaX,
                                             sigmaY, binaryThreshold);
      } else if (drawing) {
        previewTemplateImage = templateImage;
      }
    }

    cv::Mat &templateImageTmp =
        previewTemplateImage.empty() ? templateImage : previewTemplateImage;

    cv::Mat edges, mask, scaledEdges, scaledPlusEdges;
    cv::cvtColor(templateImageTmp, edges, cv::COLOR_GRAY2BGR);

    cv::threshold(templateImageTmp, mask, 0, 255, cv::THRESH_BINARY);
    edges.setTo(cv::Scalar(255, 0, 0), mask);

    if (drawing) {
      drawTo(edges, true);
    }

    cv::resize(edges, scaledEdges, sourceImage.size(), 0, 0, cv::INTER_NEAREST);
    cv::bitwise_or(scaledEdges, sourceImage, scaledPlusEdges);

    matToTexture(scaledPlusEdges, &image_tex);
  };

  generatePreviewTexture(true);

  bool save = false;

  openWindow([&](GLFWwindow *window, ImGuiIO &io) {
    int actualWidth, actualHeight;
    glfwGetWindowSize(window, &actualWidth, &actualHeight);

    bool changed = false;

    if (!io.WantCaptureMouse && detectionMode == ImageEdgeMode_Manual) {
      if (io.MouseDown[0] && mouseDownPos.x == -1) {
        mouseDownPos = {io.MousePos.x / actualWidth,
                        io.MousePos.y / actualHeight};
        mouseCurrentPos = mouseDownPos;
        changed = true;
      } else if (mouseDownPos.x != -1) {
        if (mouseCurrentPos.x != io.MousePos.x ||
            mouseCurrentPos.y != io.MousePos.y) {
          mouseCurrentPos = {io.MousePos.x / actualWidth,
                             io.MousePos.y / actualHeight};
          changed = true;
        }

        if (!io.MouseDown[0]) {
          drawTo(templateImage);
          mouseDownPos = {-1, -1};
        }
      }
    }

    ImGui::SetNextWindowFocus();
    /* ImGui::ShowDemoWindow(); */
    ImGui::Begin("Controls");

    ImGui::Text("Detection Mode:");
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Canny", &detectionMode, ImageEdgeMode_Canny);
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Threshold", &detectionMode,
                                  ImageEdgeMode_Threshold);
    ImGui::SameLine();
    changed |=
        ImGui::RadioButton("Manual", &detectionMode, ImageEdgeMode_Manual);

    ImGui::NewLine();

    if (detectionMode != ImageEdgeMode_Manual) {
      changed |= ImGui::SliderInt("Blur size", &blurSize, 0, 50);
      changed |= ImGui::SliderInt("Blur sigma X", &sigmaX, 0, 20);
      changed |= ImGui::SliderInt("Blur sigma Y", &sigmaY, 0, 20);

      ImGui::NewLine();
    }

    if (detectionMode == ImageEdgeMode_Canny) {
      changed |= ImGui::SliderInt("Canny threshold 1", &threshold1, 0, 255);
      changed |= ImGui::SliderInt("Canny threshold 2", &threshold2, 0, 255);
      changed |= ImGui::SliderInt("Join by x", &joinByX, 0, 39);
      changed |= ImGui::SliderInt("Join by y", &joinByY, 0, 39);
    } else if (detectionMode == ImageEdgeMode_Threshold) {
      changed |= ImGui::SliderInt("Binary threshold", &binaryThreshold, 0, 255);
    } else {
      // These don't change the edges, no need to set `changed`
      ImGui::Text("Tool:");
      ImGui::SameLine();
      ImGui::RadioButton("Eraser", &manualEditMode, ManualEditMode_Erase);
      ImGui::SameLine();
      ImGui::RadioButton("Line", &manualEditMode, ManualEditMode_Line);
      ImGui::SameLine();
      ImGui::RadioButton("Square", &manualEditMode, ManualEditMode_Square);
      ImGui::SameLine();
      ImGui::RadioButton("Circle", &manualEditMode, ManualEditMode_Circle);

      if (ImGui::Button("Clear")) {
        templateImage.setTo(cv::Scalar(0));
        changed = true;
      }
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

      generatePreviewTexture(false);
    }

    // Image window is full screen and non-interactive - basically, a background
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
