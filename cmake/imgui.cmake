# somewhat based on https://github.com/Pesc0/imgui-cmake/blob/master/libs/CMakeLists.txt
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
