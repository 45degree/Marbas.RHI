#include "GLFW/glfw3.h"
#include "RHIFactory.hpp"

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
  }

  return 0;
}
