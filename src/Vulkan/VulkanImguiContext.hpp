#pragma once

#include "ImguiContext.hpp"

namespace Marbas {

class VulkanImguiContext final : public ImguiContext {
 public:
  void
  Resize(uint32_t width, uint32_t height) override {}

  void
  CreateImguiContext() override {}

  void
  ClearUp() override {}

  void
  SetUpImguiBackend(GLFWwindow* windows) override {}

  void
  NewFrame() override {}

  void
  RenderData(const Semaphore* waitSemaphore, const Semaphore* signalSemaphore, uint32_t imageIndex) override {}
};

}  // namespace Marbas
