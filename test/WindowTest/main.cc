#include <gflags/gflags.h>
#include <glog/logging.h>

#include "GLFW/glfw3.h"
#include "RHIFactory.hpp"

DEFINE_string(rhi, "vulkan", "vulkan or dx12");

int
main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  int width = 800;
  int height = 600;

  glfwInit();
  std::unique_ptr<Marbas::RHIFactory> factory;
  if (FLAGS_rhi == "dx12") {
    factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::DirectX12);
  } else {
    factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  }
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
  }

  return 0;
}
