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

  int frameId = 0;
  bool playing = false;
  int fps = 5;

  initWindow(OUTPUT_WIDTH, OUTPUT_HEIGHT, "Build preview");
  GLuint image_tex;

  auto generateTexture = [&]() {
    cv::Mat image = frames.imageAt(frameId);
    matToTexture(image, &image_tex);
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

    ImGui::NewLine();

    if (ImGui::Button(playing ? "Pause" : "Play")) {
      playing = !playing;
    }
    if (playing) {
      ImGui::SameLine(0, 10);
      ImGui::SetNextItemWidth(100);
      ImGui::SliderInt("FPS", &fps, 1, 30);
    }

    ImGui::End();

    if (changed) {
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
