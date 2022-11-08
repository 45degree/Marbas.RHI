#pragma once

#include <memory>
#include <span>

#include "GLFW/glfw3.h"
#include "Swapchain.hpp"
#include "Synchronic.hpp"

namespace Marbas {

enum class RendererType {
#ifdef USE_VULKAN
  VULKAN,
#endif
#ifdef USE_D3D12
  DirectX12,
#endif
};

class RHIFactory {
 public:
  virtual void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) = 0;

 public:
  virtual Swapchain*
  GetSwapchain() = 0;

  virtual uint32_t
  AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) = 0;

  virtual int
  Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) = 0;

 public:
  virtual Fence*
  CreateFence() = 0;

  virtual void
  DestroyFence(Fence* fence) = 0;

  virtual Semaphore*
  CreateGPUSemaphore() = 0;

  virtual void
  DestroyGPUSemaphore(Semaphore* semaphore) = 0;

  // TODO: add supoort for offscreen
  // virtual void
  // OffscreenInit();
 public:
  static std::unique_ptr<RHIFactory>
  CreateInstance(const RendererType& rendererTyoe);
};

}  // namespace Marbas
