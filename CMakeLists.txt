cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/out")

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(AerialShapes)

add_executable(process_images src/process_images.cpp src/lib/detect-edge.cpp src/lib/edged-image.cpp src/lib/image-list.cpp)
add_executable(match src/match.cpp src/lib/detect-edge.cpp src/lib/edged-image.cpp src/lib/image-list.cpp)
add_executable(tests test/test.cpp)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(process_images ${OpenCV_LIBS})
target_link_libraries(match ${OpenCV_LIBS})

find_package(Boost 1.76.0 REQUIRED COMPONENTS container)
target_link_libraries(process_images Boost::container)
target_link_libraries(match Boost::container)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.6
)

FetchContent_MakeAvailable(Catch2)
target_link_libraries(tests Catch2::Catch2)

add_custom_target(
  clang-format
  COMMAND clang-format -i src/process_images.cpp src/match.cpp src/lib/detect-edge.cpp test/test.cpp
  COMMENT "Formatting with clang-format"
)
