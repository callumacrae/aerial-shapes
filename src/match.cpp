#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/edged-image.hpp"
#include "lib/frame-collection.hpp"
#include "lib/image-list.hpp"
#include "lib/mat-to-texture.hpp"
#include "lib/window.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList sourceImages = ImageList(argv[1]);
  ImageList orderedImages = sourceImages;

  FrameCollection frames;
  char frameCollectionName[50] = "";
  bool frameCollectionSaved = false;

  int width = 100;
  int height = 550;
  // Algorithm is flawed: it'll probably zoom all the way in if lineWidth > 1
  int lineWidth = 1;

  float offsetScaleStep = MATCH_OFFSET_SCALE_STEP;
  int offsetXStep = MATCH_OFFSET_X_STEP;
  int offsetYStep = MATCH_OFFSET_Y_STEP;

  float minOffsetScale = MATCH_MIN_OFFSET_SCALE;
  int maxOffset = MATCH_MAX_OFFSET;
  float whiteBias = MATCH_WHITE_BIAS;

  bool showEdges = false;
  bool showTemplate = true;

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded " << sourceImages.count() << " images from store in "
            << readElapsed.count() << "s\n";

  if (argv[2]) {
    cv::Mat templateImage = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);

    if (templateImage.empty()) {
      throw std::runtime_error("Couldn't read template image");
    }

    ImageMatch bestMatch;
    // Warning: sourceImages is managing this memory
    EdgedImage *bestMatchImage;

    sourceImages.matchTo(templateImage, &bestMatch, &bestMatchImage,
                         offsetScaleStep, offsetXStep, offsetYStep,
                         minOffsetScale, maxOffset, whiteBias);

    std::cout << "\nBest match is " << bestMatchImage->path << " with "
              << (bestMatch.percentage * 100) << "%\n";

    return 0;
  }

  initWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "Match debugger");
  GLuint image_tex;

  ImageMatch bestMatch;
  // Warning: sourceImages is managing this memory
  EdgedImage *bestMatchImage;
  int runs;
  std::chrono::duration<float> matchElapsed;
  std::chrono::duration<float> previewElapsed;

  auto generatePreviewTexture = [&]() {
    cv::Mat canvas = cv::Mat::zeros(CANVAS_HEIGHT, CANVAS_WIDTH, CV_8UC3);

    cv::Point pointA((CANVAS_WIDTH - width) / 2, (CANVAS_HEIGHT - height) / 2);
    cv::Point pointB((CANVAS_WIDTH + width) / 2, (CANVAS_HEIGHT + height) / 2);
    cv::rectangle(canvas, pointA, pointB, cv::Scalar(0, 0, 255), lineWidth);

    cv::Mat greyCanvas;
    cv::cvtColor(canvas, greyCanvas, cv::COLOR_BGR2GRAY);

    bestMatch = ImageMatch();
    bestMatchImage = nullptr;

    auto matchStart = std::chrono::high_resolution_clock::now();

    runs = sourceImages.matchTo(greyCanvas, &bestMatch, &bestMatchImage,
                                offsetScaleStep, offsetXStep, offsetYStep,
                                minOffsetScale, maxOffset, whiteBias);

    auto matchFinish = std::chrono::high_resolution_clock::now();
    matchElapsed = matchFinish - matchStart;

    auto previewStart = std::chrono::high_resolution_clock::now();

    orderedImages.sortBy("match-percentage");

    cv::Mat originalImage = bestMatchImage->getOriginal();

    if (originalImage.empty()) {
      throw std::runtime_error("Couldn't read source image");
    }

    cv::Mat sourcePlusEdges;
    if (showEdges) {
      cv::Mat greyEdges = bestMatchImage->edgesAsMatrix();
      cv::Mat edges, scaledEdges, scaledPlusEdges;
      cv::cvtColor(greyEdges, edges, cv::COLOR_GRAY2BGR);

      cv::Mat mask;
      cv::threshold(greyEdges, mask, 0, 255, cv::THRESH_BINARY);
      edges.setTo(cv::Scalar(255, 0, 0), mask);

      cv::resize(edges, scaledEdges, originalImage.size(), cv::INTER_NEAREST);
      cv::bitwise_or(scaledEdges, originalImage, scaledPlusEdges);

      float edgesAlpha = 0.9;
      cv::addWeighted(scaledPlusEdges, edgesAlpha, originalImage,
                      1.f - edgesAlpha, 0, sourcePlusEdges);
    } else {
      sourcePlusEdges = originalImage;
    }

    float realScale = (float)bestMatchImage->width / STORED_EDGES_WIDTH;

    cv::Rect roi;
    roi.x = round(bestMatch.originX * realScale);
    roi.y = round(bestMatch.originY * realScale);
    roi.width = round(CANVAS_WIDTH * realScale * bestMatch.scale);
    roi.height = round(CANVAS_HEIGHT * realScale * bestMatch.scale);

    cv::Mat cropped = sourcePlusEdges(roi);
    cv::Mat scaledImage;
    cv::resize(cropped, scaledImage, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

    cv::Mat out;
    if (showTemplate) {
      cv::Mat scaledCanvas, scaledPlusCanvas;
      cv::resize(canvas, scaledCanvas, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT),
                 cv::INTER_NEAREST);
      cv::bitwise_or(scaledCanvas, scaledImage, scaledPlusCanvas);

      float canvasAlpha = 0.6;
      cv::addWeighted(scaledPlusCanvas, canvasAlpha, scaledImage,
                      1.f - canvasAlpha, 0, out);
    } else {
      out = scaledImage;
    }

    matToTexture(out, &image_tex);

    auto previewFinish = std::chrono::high_resolution_clock::now();
    previewElapsed = previewFinish - previewStart;
  };

  generatePreviewTexture();

  openWindow([&](GLFWwindow *window, ImGuiIO &io) {
    bool changed = false;

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Controls");

    changed |= ImGui::SliderInt("Width", &width, 0, CANVAS_WIDTH + 50);
    changed |= ImGui::SliderInt("Height", &height, 0, CANVAS_HEIGHT + 50);
    /* changed |= ImGui::SliderInt("Line width", &lineWidth, 0, 50); */

    ImGui::NewLine();

    changed |=
        ImGui::SliderFloat("Offset scale step", &offsetScaleStep, 0.05, 0.5);
    changed |= ImGui::SliderInt("Offset x step", &offsetXStep, 1, 20);
    changed |= ImGui::SliderInt("Offset y step", &offsetYStep, 1, 20);
    changed |= ImGui::SliderFloat("Min offset scale", &minOffsetScale, 0, 0.9);
    changed |= ImGui::SliderInt("Max offset", &maxOffset, 1, 50);
    changed |= ImGui::SliderFloat("White bias", &whiteBias, 0, 1);

    ImGui::NewLine();

    changed |= ImGui::Checkbox("Show edges?", &showEdges);
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Show template?", &showTemplate);

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
      ImGui::Text("Match timer: %.2fs (%.2fs avg)", matchElapsed.count(),
                  matchElapsed.count() / orderedImages.count());
      ImGui::Text("Preview timer: %.2fs", previewElapsed.count());

      ImGui::TreePop();
    }

    if (ImGui::TreeNode("All matches")) {
      if (ImGui::BeginChild("matches")) {
        for (auto &image : orderedImages) {
          ImGui::Text("%.1f%%: %s", image->lastMatch.percentage * 100,
                      image->path.c_str());
        }
      }
      ImGui::EndChild();
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Build")) {
      ImGui::Text("Currently %i frames", (int)frames.size());

      if (ImGui::Button("Add frame")) {
        frames.addFrame(sourceImages);
        frameCollectionSaved = false;
      }
      ImGui::SameLine();
      if (ImGui::Button("Remove last frame")) {
        frames.popFrame();
        frameCollectionSaved = false;
      }

      ImGui::NewLine();

      if (ImGui::InputText("Name", frameCollectionName, 50)) {
        frameCollectionSaved = false;
      }

      if (frames.size() && strlen(frameCollectionName)) {
        if (ImGui::Button("Save")) {
          std::string name(frameCollectionName);
          frames.save(name);

          frameCollectionSaved = true;
        }
      }

      if (frameCollectionSaved) {
        ImGui::SameLine();
        ImGui::Text("Saved");
      }

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
