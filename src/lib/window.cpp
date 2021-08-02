#include "window.hpp"

GLFWwindow* window;

static void glfw_error_callback(int error, const char* description) {
  char errorText[50];
  sprintf(errorText, "Glfw error %i: %s", error, description);
  throw std::runtime_error(errorText);
}

void initWindow(int width, int height, const char* title) {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit()) {
    throw std::runtime_error("Error initiating glfw");
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);      // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);      // 3.0+ only
#endif

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);

  // Create window with graphics context
  int attemptWidth = std::min(mode->width, width);
  int attemptHeight = std::min(mode->height, height);
  window = glfwCreateWindow(attemptWidth, attemptHeight, title, NULL, NULL);

  if (window == NULL) {
    throw std::runtime_error("Error initiating window");
  }

  int actualWidth, actualHeight;
  glfwGetWindowSize(window, &actualWidth, &actualHeight);

  if (actualWidth < width || actualHeight < height) {
    float scale = fmin((float)actualWidth / width, (float)actualHeight / height);
    width *= scale;
    height *= scale;
  }

  glfwSetWindowSize(window, width, height);
  glfwSetWindowAspectRatio(window, width, height);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
  bool err = gl3wInit() != 0;
  if (err) {
    throw std::runtime_error("Failed to initialize OpenGL loader!");
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

// todo pass onFrame by reference?
void openWindow(const onFrameFn onFrame) {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool shouldClose = onFrame(window);
    if (shouldClose) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
    }

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.6f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}
