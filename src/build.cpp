#include "precompiled.h"

#include "config.h"

#include "lib/frame-collection.hpp"
#include "lib/mat-to-texture.hpp"
#include "lib/window.hpp"

int main(int argc, const char *argv[]) {
  std::cout << "Preloading frames…\n";

  auto lastFrame = std::chrono::high_resolution_clock::now();

  std::string name(argv[1]);
  FrameCollection frames(name);
  frames.preloadAll();

  auto loadFinish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> loadDuration = loadFinish - lastFrame;

  std::cout << "Preloaded frames in " << loadDuration.count() << "s\n";

  if (argv[2] && strcmp(argv[2], "--write") == 0) {
    auto writeBegin = std::chrono::high_resolution_clock::now();

    frames.writeImages(name);

    auto writeFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> writeDuration = writeFinish - writeBegin;
    std::cout << "Written " << frames.size() << " frames to disk in "
      << writeDuration.count() << "s\n";

    return 0;
  }

  bool edited = false;
  bool saved = false;
  int frameId = 0;
  bool playing = false;
  int fps = 5;

  initWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "Build preview");
  GLuint image_tex;
  std::vector<MatchData>::iterator matchForFrame;
  std::vector<MatchData> *matchesForFrame;
  bool doPreview = false;
  std::vector<MatchData>::iterator previewMatch;

  auto generateTexture = [&]() {
    cv::Mat image = doPreview ? frames.imageFor(previewMatch) : frames.imageAt(frameId);
    doPreview = false;
    matToTexture(image, &image_tex);
    matchForFrame = frames.matchAt(frameId);
    matchesForFrame = frames.matchesAt(frameId);
  };

  generateTexture();

  openWindow([&](GLFWwindow *window, ImGuiIO &io) {
    bool changed = false;

    if (playing) {
      auto current = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> delta = current - lastFrame;
      if (delta.count() > 1.f / fps) {
        frameId = frameId == frames.size() - 1 ? 0 : frameId + 1;
        lastFrame = current;

        changed = true;
      }
    }

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Controls");

    changed |= ImGui::SliderInt("Frame ID", &frameId, 0, frames.size() - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton("-") && frameId > 0) {
      --frameId;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("+") && frameId < frames.size() - 1) {
      ++frameId;
      changed = true;
    }

    ImGui::NewLine();

    if (ImGui::Button(playing ? "Pause" : "Play")) {
      playing = !playing;
    }
    if (playing) {
      ImGui::SameLine(0, 10);
      ImGui::SetNextItemWidth(100);
      ImGui::SliderInt("FPS", &fps, 1, 30);
    }

    ImGui::NewLine();

    if (edited || saved) {
      if (edited && ImGui::Button("Save changes")) {
        frames.save(name);
        edited = false;
        saved = true;
      } else if (saved) {
        ImGui::Text("Saved!");
      }

      ImGui::NewLine();
    }

    if (ImGui::TreeNode("Reposition")) {
      ImGui::Text("Scale: %.2f", matchForFrame->scale);
      ImGui::SameLine();
      if (ImGui::SmallButton("-##scale-minus")) {
        frames.editMatchScale(frameId, matchForFrame->scale + 0.01);
        changed = true;
        edited = true;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("+##scale-plus")) {
        frames.editMatchScale(frameId, matchForFrame->scale - 0.01);
        changed = true;
        edited = true;
      }

      ImGui::Text("x: %i", matchForFrame->originX);
      ImGui::SameLine();
      if (ImGui::SmallButton("-##x-minus")) {
        frames.editMatchOriginX(frameId, matchForFrame->originX - 1);
        changed = true;
        edited = true;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("+##x-plus")) {
        frames.editMatchOriginX(frameId, matchForFrame->originX + 1);
        changed = true;
        edited = true;
      }

      ImGui::Text("y: %i", matchForFrame->originY);
      ImGui::SameLine();
      if (ImGui::SmallButton("-##y-minus")) {
        frames.editMatchOriginY(frameId, matchForFrame->originY - 1);
        changed = true;
        edited = true;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("+##y-plus")) {
        frames.editMatchOriginY(frameId, matchForFrame->originY + 1);
        changed = true;
        edited = true;
      }

      ImGui::NewLine();
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("All matches")) {
      if (ImGui::BeginChild("matches")) {
        for (std::vector<MatchData>::iterator match = matchesForFrame->begin();
            match != matchesForFrame->end(); ++match) {
          ImGui::PushID(match->path.c_str());
          if (ImGui::SmallButton("Preview")) {
            previewMatch = match;
            doPreview = true;
            changed = true;
          }
          ImGui::SameLine();
          if (ImGui::SmallButton("y")) {
            frames.forceMatch(frameId, match);
            changed = true;
            // Stop usages of match below segfaulting
            match = matchesForFrame->begin();
            edited = true;
          }
          ImGui::SameLine();
          if (ImGui::SmallButton("n")) {
            frames.removeMatch(frameId, match);
            edited = true;
          }
          ImGui::SameLine();
          ImGui::Text("%.1f%%: %s", match->percentage * 100, match->path.c_str());
          ImGui::SameLine();
          if (ImGui::SmallButton("copy")) {
            ImGui::SetClipboardText(match->path.c_str());
          }
          ImGui::PopID();
        }
      }
      ImGui::EndChild();
      ImGui::TreePop();
    }

    ImGui::End();

    if (changed) {
      saved = false;
      generateTexture();
    }

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
}
