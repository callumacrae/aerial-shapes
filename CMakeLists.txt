cmake_minimum_required(VERSION 3.14)

include(FetchContent)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/out")

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(AerialShapes)

add_library(LibraryFiles
  src/lib/detect-edge.cpp
  src/lib/edged-image.cpp
  src/lib/edit-image-edges.cpp
  src/lib/image-list.cpp
  src/lib/mat-to-texture.cpp
  src/lib/window.cpp)
add_executable(process_images src/process_images.cpp)
add_executable(match src/match.cpp)
target_link_libraries(process_images LibraryFiles)
target_link_libraries(match LibraryFiles)

target_precompile_headers(process_images PRIVATE src/precompiled.h)
target_precompile_headers(match REUSE_FROM process_images)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(LibraryFiles ${OpenCV_LIBS})

find_package(Boost 1.76.0 REQUIRED COMPONENTS container)
target_link_libraries(LibraryFiles Boost::container)

include(cmake/imgui.cmake)
target_link_libraries(LibraryFiles glfw IMGUI GL3W)

FetchContent_Declare(
  linenoise
  GIT_REPOSITORY https://github.com/yhirose/cpp-linenoise
)

FetchContent_MakeAvailable(linenoise)
target_link_libraries(process_images linenoise)

add_custom_target(
  clang-format
  COMMAND clang-format -i src/process_images.cpp src/match.cpp ${LibraryFiles}
  COMMENT "Formatting with clang-format"
)
