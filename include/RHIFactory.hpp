#pragma once

#include <memory>
#include <span>

#include "BufferContext.hpp"
#include "GLFW/glfw3.h"
#include "PipelineContext.hpp"
#include "Swapchain.hpp"
#include "Synchronic.hpp"

namespace Marbas {

enum class RendererType {
  VULKAN,
  DirectX12,
};

class RHIFactory {
 public:
  virtual void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) = 0;

  virtual void
  Quit() = 0;

 public:
  virtual Swapchain*
  GetSwapchain() = 0;

  virtual uint32_t
  AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) = 0;

  virtual int
  Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) = 0;

  virtual void
  WaitIdle() = 0;

 public:
  virtual Fence*
  CreateFence() = 0;

  virtual void
  WaitForFence(Fence* fence) = 0;

  virtual void
  ResetFence(Fence* fence) = 0;

  virtual void
  DestroyFence(Fence* fence) = 0;

  virtual Semaphore*
  CreateGPUSemaphore() = 0;

  virtual void
  DestroyGPUSemaphore(Semaphore* semaphore) = 0;

 public:
  PipelineContext*
  GetPipelineContext() {
    return m_pipelineContext.get();
  }

  BufferContext*
  GetBufferContext() {
    return m_bufferContext.get();
  }

  // TODO: add supoort for offscreen
  // virtual void
  // OffscreenInit();
 public:
  static std::unique_ptr<RHIFactory>
  CreateInstance(const RendererType& rendererTyoe);

  std::unique_ptr<PipelineContext> m_pipelineContext = nullptr;
  std::unique_ptr<BufferContext> m_bufferContext = nullptr;
};

}  // namespace Marbas
