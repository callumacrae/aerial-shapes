#include <linenoise.hpp>

#include "precompiled.h"

#include "config.h"

#include "lib/detect-edge.hpp"
#include "lib/edit-image-edges.hpp"
#include "lib/image-list.hpp"

std::optional<int> imageFromArg(ImageList &imageList, const std::string &arg) {
  if (arg.empty()) {
    std::cerr << "Argument required\n";
    return {};
  }

  int id = -1;
  try {
    id = stoi(arg);
  } catch (std::invalid_argument) {
    for (size_t i = 0; i < imageList.count(); ++i) {
      if (imageList.at(i)->path == arg) {
        id = i;
        break;
      }
    }
  }

  if (id == -1) {
    std::cerr << "Image not found\n";
    return {};
  }

  if (id > imageList.count() - 1) {
    std::cerr << "That image doesn't exist: highest ID is "
      << (imageList.count() - 1) << '\n';
    return {};
  }

  return id;
}

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
    } else if (command == "sync") {
      auto imageListBackup = imageList;

      int newImages = imageList.sync();

      if (!newImages) {
        std::cout << "Synced: no new images found\n";
        continue;
      }

      char prompt[35];
      sprintf(prompt, "%i new images found, save? (Y/n) ", newImages);

      std::string line;
      auto quit = linenoise::Readline(prompt, line);

      if (quit) {
        break;
      }

      if (line != "n") {
        std::cout << "Saving " << newImages << " new images to store\n";
        imageList.save();
      } else {
        imageList = imageListBackup;
      }
    } else if (command == "ls") {
      std::cout << imageList.count() << " images in store:\n\n";

      int i = 0;
      for (const std::shared_ptr<EdgedImage> &image : imageList) {
        std::cout << i << ": " << image->path << " (" << image->width << "x"
                  << image->height << ")\n";
        i++;
      }

      std::cout << '\n';
    } else if (command == "edit") {
      auto maybeImage = imageFromArg(imageList, std::string(arg));

      if (!maybeImage.has_value()) {
        continue;
      }

      std::shared_ptr<EdgedImage> &image = imageList.at(maybeImage.value());

      std::cout << "Editing: " << image->path << "\n";

      std::optional<EdgedImage *> maybeNewImage = editImageEdges(*image);

      if (maybeNewImage.has_value()) {
        image = std::shared_ptr<EdgedImage>(maybeNewImage.value());
        imageList.save();
        std::cout << "Changes saved\n";
      } else {
        std::cout << "Changes discarded\n";
      }
    } else if (command == "rm") {
      auto maybeImage = imageFromArg(imageList, std::string(arg));

      if (!maybeImage.has_value()) {
        continue;
      }

      int id = maybeImage.value();
      std::string path = imageList.at(id)->path;

      std::cout << "Removing: " << path << "\n";

      imageList.erase(id);
      imageList.save();

      char prompt[60];
      sprintf(prompt, "Image removed from store: delete file from disk? (Y/n) ");

      std::string line;
      auto quit = linenoise::Readline(prompt, line);

      if (quit) {
        break;
      }

      if (line != "n") {
        std::filesystem::remove(path);
        std::cout << "File removed\n";
      }
    } else if (command == "sort") {
      imageList.sortBy("path");
      std::cout << "Sorted by file path - this will not be saved to store\n";
    } else if (command == "save") {
      imageList.save(false);
      std::cout << "Store saved\n";
    } else {
      std::cout << "?\n";
    }
  }
}
