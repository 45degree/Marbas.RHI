#pragma once

#include <GLFW/glfw3.h>

#include "Synchronic.hpp"

namespace Marbas {

class ImguiContext {
 public:
  virtual void
  Resize(uint32_t width, uint32_t height) = 0;

  virtual void
  CreateImguiContext() = 0;

  virtual void
  ClearUp() = 0;

  virtual void
  SetUpImguiBackend(GLFWwindow* windows) = 0;

  virtual void
  NewFrame() = 0;

  virtual void
  RenderData(const Semaphore* waitSemaphore, const Semaphore* signalSemaphore, uint32_t imageIndex) = 0;
};

}  // namespace Marbas
