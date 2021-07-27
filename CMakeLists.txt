cmake_minimum_required(VERSION 3.14)

include(FetchContent)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/out")

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(AerialShapes)

set(LibraryFiles
  src/lib/detect-edge.cpp
  src/lib/edged-image.cpp
  src/lib/edit-image-edges.cpp
  src/lib/image-list.cpp
  src/lib/mat-to-texture.cpp
  src/lib/window.cpp)
add_executable(process_images src/process_images.cpp ${LibraryFiles})
add_executable(match src/match.cpp ${LibraryFiles})
add_executable(tests test/test.cpp)

target_precompile_headers(process_images PRIVATE src/precompiled.h)
target_precompile_headers(match REUSE_FROM process_images)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(process_images ${OpenCV_LIBS})
target_link_libraries(match ${OpenCV_LIBS})

find_package(Boost 1.76.0 REQUIRED COMPONENTS container)
target_link_libraries(process_images Boost::container)
target_link_libraries(match Boost::container)

# IMGUI CONFIG START: https://github.com/Pesc0/imgui-cmake/blob/master/libs/CMakeLists.txt
find_package(OpenGL REQUIRED)

# Use gl3w as GL loader. It is the default in imgui's examples.
# todo: use OpenGL::GL?
set(GL3W_DIR "extern/imgui/examples/libs/gl3w")
add_library(GL3W STATIC)

target_sources(GL3W PRIVATE ${GL3W_DIR}/GL/gl3w.c)
target_include_directories(GL3W PUBLIC ${GL3W_DIR})
target_link_libraries(GL3W PUBLIC ${OPENGL_LIBRARIES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_library(IMGUI STATIC)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)

set(IMGUI_DIR "extern/imgui")

file(GLOB imgui_source_files "${IMGUI_DIR}/*.cpp")

target_sources( IMGUI
                PRIVATE ${imgui_source_files}

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            )


target_compile_definitions(IMGUI PUBLIC -DIMGUI_IMPL_OPENGL_LOADER_GL3W)
target_link_libraries(IMGUI PUBLIC glfw GL3W ${CMAKE_DL_LIBS})
# IMGUI CONFIG END

target_link_libraries(process_images glfw IMGUI GL3W)
target_link_libraries(match glfw IMGUI GL3W)

FetchContent_Declare(
  linenoise
  GIT_REPOSITORY https://github.com/yhirose/cpp-linenoise
)

FetchContent_MakeAvailable(linenoise)
target_link_libraries(process_images linenoise)

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
