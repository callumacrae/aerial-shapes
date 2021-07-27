#include <linenoise.hpp>

#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/edit-image-edges.hpp"
#include "lib/image-list.hpp"

int main(int argc, const char *argv[]) {
  auto readStart = std::chrono::high_resolution_clock::now();

  if (!argv[1]) {
    std::cerr << "No directory specified, exiting\n";
    exit(1);
  }

  std::cout << std::fixed << std::setprecision(2);

  ImageList imageList(argv[1]);

  if (imageList.count()) {
    auto readFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> readElapsed = readFinish - readStart;

    std::cout << "Loaded " << imageList.count() << " images from store in "
              << readElapsed.count() << "s\n";
  } else {
    std::cout << "No store found for this directory, type \"generate\" to "
              << "generate one\n";
  }

  while (true) {
    std::cout << '\n'; // This bugs out if added to the linenoise call
    std::string line;
    auto quit = linenoise::Readline("> ", line);

    if (quit) {
      break;
    }

    std::string_view query(line);
    auto splitPos = query.find(' ');
    std::string_view command = query.substr(0, splitPos);
    std::string_view arg;

    if (splitPos != query.npos) {
      arg = query.substr(query.find(' ') + 1);
    }

    linenoise::AddHistory(line.c_str());

    if (command == "exit" || command == "q") {
      std::cout << "bye\n";
      break;
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (command == "generate" || command == "reset") {
      if (command == "generate" && imageList.count() > 0) {
        std::cerr
            << "Store has already been generated: use \"reset\" to reset\n";
        continue;
      } else if (command == "reset") {
        std::cout << '\n';
        std::string line;
        auto quit =
            linenoise::Readline("Are you sure you wish to reset? (y/N) ", line);

        if (quit) {
          break;
        }

        if (line != "y") {
          std::cout << "Aborting reset\n";
          continue;
        }
      }

      imageList.generate();
      imageList.save();

      auto finish = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> elapsed = finish - start;
      std::cout << "Generated in " << elapsed.count() << "s\n";
    } else if (command == "ls") {
      std::cout << imageList.count() << " images in store:\n\n";

      int i = 0;
      for (const EdgedImage &image : imageList) {
        std::cout << i << ": " << image.path << " (" << image.width << "x"
                  << image.height << ")\n";
        i++;
      }

      std::cout << '\n';
    } else if (command == "edit") {
      if (arg.empty()) {
        std::cerr << "edit needs argument\n";
        continue;
      }

      // todo figure out a better initialiser lol
      std::vector<EdgedImage>::reference image = imageList.at(0);
      if (arg.find('/') != arg.npos) {
        std::cerr << "This isn't supported yet soz\n";
      } else {
        int id = stoi(std::string(arg));
        image = imageList.at(id);
      }

      std::cout << "Editing: " << image.path << "\n";

      std::optional<EdgedImage> maybeEdgedImage = editImageEdges(image);

      if (maybeEdgedImage.has_value()) {
        image = maybeEdgedImage.value();
        imageList.save();
        std::cout << "Changes saved\n";
      } else {
        std::cout << "Changed discarded\n";
      }
    }
  }
}
