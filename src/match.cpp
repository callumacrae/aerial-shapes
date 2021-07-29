#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/edged-image.hpp"
#include "lib/image-list.hpp"
#include "lib/mat-to-texture.hpp"
#include "lib/window.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList sourceImages = ImageList(argv[1]);

  int width = 100;
  int height = 550;
  int lineWidth = 4;
  float whiteBias = 0.75;

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded " << sourceImages.count() << " images from store in "
            << readElapsed.count() << "s\n";

  if (argv[2]) {
    cv::Mat templateImage = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);

    if (templateImage.empty()) {
      std::cerr << "Couldn't read template image\n";
      return 1;
    }

    ImageMatch bestMatch;
    EdgedImage *bestMatchImage = nullptr;

    for (EdgedImage &sourceImage : sourceImages) {
      ImageMatch match;
      sourceImage.matchTo(templateImage, &match);
      std::cout << sourceImage.path << " match: " << (match.percentage * 100)
                << "%\n";

      if (match.percentage > bestMatch.percentage) {
        bestMatch = match;
        bestMatchImage = &sourceImage;
      }
    }

    std::cout << "\nBest match is " << bestMatchImage->path << " with "
              << (bestMatch.percentage * 100) << "%\n";

    return 0;
  }

  initWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "Match debugger");
  GLuint image_tex;

  ImageMatch bestMatch;
  EdgedImage *bestMatchImage = nullptr;
  int runs;

  auto generatePreviewTexture = [&]() {
    cv::Mat canvas = cv::Mat::zeros(CANVAS_HEIGHT, CANVAS_WIDTH, CV_8UC3);

    cv::Point pointA((CANVAS_WIDTH - width) / 2, (CANVAS_HEIGHT - height) / 2);
    cv::Point pointB((CANVAS_WIDTH + width) / 2, (CANVAS_HEIGHT + height) / 2);
    cv::rectangle(canvas, pointA, pointB, cv::Scalar(0, 0, 255), lineWidth);

    cv::Mat greyCanvas;
    cv::cvtColor(canvas, greyCanvas, cv::COLOR_BGR2GRAY);

    bestMatch = ImageMatch();
    bestMatchImage = nullptr;
    runs = 0;

    for (EdgedImage &sourceImage : sourceImages) {
      ImageMatch match;
      runs += sourceImage.matchTo(greyCanvas, &match, whiteBias);
      if (match.percentage > bestMatch.percentage) {
        bestMatch = match;
        bestMatchImage = &sourceImage;
      }
    }

    cv::Mat originalImage = cv::imread(bestMatchImage->path);

    if (originalImage.empty()) {
      std::cerr << "Couldn't read image\n";
      exit(1);
    }

    cv::Mat greyEdges = bestMatchImage->edgesAsMatrix();
    cv::Mat edges, scaledEdges, scaledPlusEdges;
    cv::cvtColor(greyEdges, edges, cv::COLOR_GRAY2BGR);

    cv::Mat mask;
    cv::threshold(greyEdges, mask, 0, 255, cv::THRESH_BINARY);
    edges.setTo(cv::Scalar(255, 0, 0), mask);

    cv::resize(edges, scaledEdges, originalImage.size());
    cv::bitwise_or(scaledEdges, originalImage, scaledPlusEdges);

    cv::Mat sourcePlusEdges;
    float edgesAlpha = 0.9;
    cv::addWeighted(scaledPlusEdges, edgesAlpha, originalImage, 1.f - edgesAlpha,
        0, sourcePlusEdges);

    float realScale = (float)bestMatchImage->width / STORED_EDGES_WIDTH;

    cv::Rect roi;
    roi.x = round(bestMatch.originX * realScale);
    roi.y = round(bestMatch.originY * realScale);
    roi.width = round(CANVAS_WIDTH * realScale * bestMatch.scale);
    roi.height = round(CANVAS_HEIGHT * realScale * bestMatch.scale);

    cv::Mat cropped = sourcePlusEdges(roi);
    cv::Mat scaledImage;
    cv::resize(cropped, scaledImage, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

    cv::Mat scaledCanvas, scaledPlusCanvas;
    cv::resize(canvas, scaledCanvas, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));
    cv::bitwise_or(scaledCanvas, scaledImage, scaledPlusCanvas);

    cv::Mat out;
    float canvasAlpha = 0.6;
    cv::addWeighted(scaledPlusCanvas, canvasAlpha, scaledImage, 1.f - canvasAlpha,
        0, out);

    matToTexture(out, &image_tex);
  };

  generatePreviewTexture();

  openWindow([&](GLFWwindow* window) {
    bool changed = false;

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Controls");

    changed |= ImGui::SliderInt("Width", &width, 0, CANVAS_WIDTH + 50);
    changed |= ImGui::SliderInt("Height", &height, 0, CANVAS_HEIGHT + 50);
    changed |= ImGui::SliderInt("Line width", &lineWidth, 0, 50);
    changed |= ImGui::SliderFloat("White bias", &whiteBias, 0, 1);

    ImGui::NewLine();

    if (ImGui::TreeNode("Match info")) {
      float child_height = ImGui::GetTextLineHeight();

      if (ImGui::BeginChild("path", ImVec2(0, child_height))) {
        ImGui::Text("Best match: %s", bestMatchImage->path.c_str());
      }
      ImGui::EndChild();
      ImGui::Text("%% match: %.1f%%", bestMatch.percentage * 100);
      ImGui::Text("Scale: %.2f", bestMatch.scale);
      ImGui::Text("Offset: (%i,%i)", bestMatch.originX, bestMatch.originY);
      ImGui::Text("Runs: %i", runs);
      ImGui::TreePop();
    }

    ImGui::End();

    if (changed) {
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
}
