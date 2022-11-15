#pragma once

#include <GLFW/glfw3.h>

#include "Synchronic.hpp"

namespace Marbas {

struct ImguiRenderDataInfo {
  const Semaphore* waitSemaphore = nullptr;
  const Semaphore* signalSemaphore = nullptr;
};

class ImguiContext {
 public:
  virtual void
  Resize(uint32_t width, uint32_t height) = 0;

  virtual void
  ClearUp() = 0;

  virtual void
  SetUpImguiBackend(GLFWwindow* windows) = 0;

  virtual void
  NewFrame() = 0;

  virtual void
  RenderData(uint32_t imageIndex, const ImguiRenderDataInfo& renderInfo) = 0;
};

}  // namespace Marbas
