#pragma once

#include "GLFW/glfw3.h"
#include "Swapchain.hpp"

namespace Marbas {

class RHIFactory {
 public:
  virtual void
  GetSwapchain(Swapchain& swapchain) = 0;

  virtual void
  Init(GLFWwindow* window, bool isOffscreen) = 0;
};

}  // namespace Marbas
