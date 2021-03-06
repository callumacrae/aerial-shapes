#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/edged-image.hpp"
#include "lib/frame-collection.hpp"
#include "lib/image-list.hpp"
#include "lib/mat-to-texture.hpp"
#include "lib/window.hpp"

enum TemplateShapes { TemplateShape_Rect, TemplateShape_Circle };

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  ImageList sourceImages = ImageList(argv[1]);
  ImageList orderedImages = sourceImages;

  FrameCollection frames;
  char frameCollectionName[50] = "";
  bool frameCollectionSaved = false;

  int shape = TemplateShape_Rect;
  int width = 48;
  int height = 550;
  // Algorithm is flawed: it'll probably zoom all the way in if lineWidth > 1
  int lineWidth = 1;
  int templateOffsetX = 0;
  int templateOffsetY = 0;

  float offsetScaleStep = MATCH_OFFSET_SCALE_STEP;
  int offsetXStep = MATCH_OFFSET_X_STEP;
  int offsetYStep = MATCH_OFFSET_Y_STEP;

  float minOffsetScale = MATCH_MIN_OFFSET_SCALE;
  int maxOffset = MATCH_MAX_OFFSET;
  float whiteBias = MATCH_WHITE_BIAS;

  bool showEdges = false;
  bool showTemplate = true;

  bool updateWhenChanged = true;

  auto readFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> readElapsed = readFinish - readStart;

  std::cout << "Loaded " << sourceImages.count() << " images from store in "
            << readElapsed.count() << "s\n";

  initWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "Match debugger");
  GLuint image_tex;

  ImageMatch bestMatch;
  // Warning: sourceImages is managing this memory
  EdgedImage *bestMatchImage;
  EdgedImage *previewImage;
  int runs;
  std::chrono::duration<float> matchElapsed;
  std::chrono::duration<float> previewElapsed;

  auto generatePreviewTexture = [&]() {
    cv::Mat canvas = cv::Mat::zeros(CANVAS_HEIGHT, CANVAS_WIDTH, CV_8UC3);

    cv::Point center(CANVAS_WIDTH / 2 + templateOffsetX,
                     CANVAS_HEIGHT / 2 + templateOffsetY);
    if (shape == TemplateShape_Rect) {
      cv::Point diff(width / 2, height / 2);
      cv::rectangle(canvas, center - diff, center + diff, cv::Scalar(0, 0, 255),
                    lineWidth);
    } else if (shape == TemplateShape_Circle) {
      cv::circle(canvas, center, width / 2, cv::Scalar(0, 0, 255), lineWidth);
    }

    cv::Mat greyCanvas;
    cv::cvtColor(canvas, greyCanvas, cv::COLOR_BGR2GRAY);

    ImageMatch oldBestMatch = bestMatch;
    EdgedImage *oldBestMatchImage = bestMatchImage;

    bestMatch = ImageMatch();
    bestMatchImage = nullptr;

    if (previewImage != nullptr) {
      bestMatch = previewImage->lastMatch;
      bestMatchImage = previewImage;
      previewImage = nullptr;
      matchElapsed = std::chrono::seconds(0);
    } else {
      auto matchStart = std::chrono::high_resolution_clock::now();

      sourceImages.provideMatchContext(templateOffsetX, templateOffsetY);
      runs = sourceImages.matchTo(greyCanvas, &bestMatch, &bestMatchImage,
                                  offsetScaleStep, offsetXStep, offsetYStep,
                                  minOffsetScale, maxOffset, whiteBias);

      auto matchFinish = std::chrono::high_resolution_clock::now();
      matchElapsed = matchFinish - matchStart;

      orderedImages.sortBy("match-percentage");

      // This displays the wrong image but stops a segfault
      if (!bestMatchImage) {
        bestMatch = oldBestMatch;
        bestMatchImage = oldBestMatchImage;
      }
    }

    auto previewStart = std::chrono::high_resolution_clock::now();

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

    cv::Mat scaledImage;
    try {
      cv::Mat cropped = sourcePlusEdges(roi);
      cv::resize(cropped, scaledImage, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));
    } catch (cv::Exception) {
      // This sucks but is better than crashing the programme
      scaledImage = cv::Mat::zeros(cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT),
                                   sourcePlusEdges.type());
    }

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

    changed |= ImGui::RadioButton("Rectangle", &shape, TemplateShape_Rect);
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Circle", &shape, TemplateShape_Circle);
    changed |= ImGui::SliderInt("Width", &width, 0, CANVAS_WIDTH + 50);
    ImGui::SameLine();
    if (ImGui::SmallButton("-##lesswidth")) {
      width--;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("+##morewidth")) {
      width++;
      changed = true;
    }
    if (shape == TemplateShape_Rect) {
      changed |= ImGui::SliderInt("Height", &height, 0, CANVAS_HEIGHT + 50);
    }
    /* changed |= ImGui::SliderInt("Line width", &lineWidth, 0, 50); */
    changed |= ImGui::SliderInt("Offset X", &templateOffsetX, -100, 100);
    ImGui::SameLine();
    if (ImGui::SmallButton("-##lessoffx")) {
      templateOffsetX--;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("+##moreoffx")) {
      templateOffsetX++;
      changed = true;
    }
    changed |= ImGui::SliderInt("Offset Y", &templateOffsetY, -100, 100);
    ImGui::SameLine();
    if (ImGui::SmallButton("-##lessoffy")) {
      templateOffsetY--;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("+##moreoffy")) {
      templateOffsetY++;
      changed = true;
    }

    ImGui::NewLine();

    changed |=
        ImGui::SliderFloat("Offset scale step", &offsetScaleStep, 0.025, 0.5);
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

    // This only works when below the controls
    ImGui::Checkbox("Update when changed?", &updateWhenChanged);
    if (!updateWhenChanged) {
      changed = false;
      ImGui::SameLine();
      if (ImGui::SmallButton("Update")) {
        changed = true;
      }
    }

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
          ImGui::PushID(image->path.c_str());
          if (ImGui::SmallButton("Preview")) {
            previewImage = image.get();
            changed = true;
          }
          ImGui::SameLine();
          ImGui::Text("%.1f%%: %s", image->lastMatch.percentage * 100,
                      image->path.c_str());
          ImGui::SameLine();
          if (ImGui::SmallButton("copy")) {
            ImGui::SetClipboardText(image->path.c_str());
          }
          ImGui::PopID();
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

      if (strlen(frameCollectionName)) {
        if (frames.size()) {
          if (ImGui::Button("Save")) {
            std::string name(frameCollectionName);
            frames.save(name);

            frameCollectionSaved = true;
          }
        } else {
          if (ImGui::Button("Load")) {
            frames = FrameCollection(frameCollectionName);
          }
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
