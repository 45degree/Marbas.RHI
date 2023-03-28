#include "GLFW/glfw3.h"
#include "RHIFactory.hpp"

struct CustomContext {
  int a = 10;

  int
  getValue() {
    a++;
    return a;
  }
};

struct CustomExtension {
  using ContextInterface = CustomContext;

  constexpr static std::string_view name = "custom Extension";

  static std::unique_ptr<ContextInterface>
  CreateVulkanContext() {
    return std::make_unique<ContextInterface>();
  }
};

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  factory->Registry<CustomExtension>();

  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);
  factory->Init(glfwWindow, width, height);

  auto extensionContext = factory->GetContext<CustomExtension>();
  assert(extensionContext != nullptr);
  assert(extensionContext->getValue() == 11);

  return 0;
}
