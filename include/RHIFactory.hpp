#pragma once

#include "GLFW/glfw3.h"
#include "Swapchain.hpp"

namespace Marbas {

class RHIFactory {
 public:
  virtual void
  GetSwapchain(Swapchain& swapchain) = 0;

  virtual void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) = 0;

  // TODO: add supoort for offscreen
  // virtual void
  // OffscreenInit();
};

}  // namespace Marbas
