#pragma once

#include "Swapchain.hpp"

namespace Marbas {

class RHIFactory {
 public:
  virtual void
  GetSwapchain(Swapchain& swapchain);
};

}  // namespace Marbas
