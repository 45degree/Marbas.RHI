/**
 * Copyright 2022.11.30 45degree
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <memory>
#include <span>
#include <type_traits>
#include <unordered_map>

#include "BufferContext.hpp"
#include "GLFW/glfw3.h"
#include "ImguiContext.hpp"
#include "PipelineContext.hpp"
#include "Swapchain.hpp"
#include "Synchronic.hpp"

namespace Marbas {

enum class RendererType {
  VULKAN,
  DirectX12,
};

class RHIFactory {
  struct ExtensionInterface {
    virtual void
    Init(RendererType type) = 0;

    virtual void
    Destroy() = 0;
  };

  template <typename T>
  struct ExtensionPlugin : public ExtensionInterface {
    using Context = typename T::ContextInterface;
    std::unique_ptr<Context> m_member;

    void
    Init(RendererType type) override {
      if (type == RendererType::VULKAN) {
        m_member = T::CreateVulkanContext();
      }
    }

    void
    Destroy() override {
      m_member = nullptr;
    }
  };

 public:
  RHIFactory() = default;
  virtual ~RHIFactory() = default;

 public:
  virtual void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) = 0;

  virtual void
  Quit() = 0;

 public:
  virtual std::vector<SampleCount>
  FindSupportSampleCount() = 0;

 public:
  virtual Swapchain*
  GetSwapchain() = 0;

  virtual Swapchain*
  RecreateSwapchain(Swapchain* oldSwapchain, uint32_t width, uint32_t height) = 0;

  virtual int
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

  ImguiContext*
  GetImguiContext() {
    return m_imguiContext.get();
  }

 public:
  template <typename Extension>
  void
  Registry() {
    m_extensions.insert({Extension::name, std::make_unique<ExtensionPlugin<Extension>>()});
  }

  template <typename Extension>
  typename Extension::ContextInterface*
  GetContext() {
    if (m_extensions.find(Extension::name) == m_extensions.end()) {
      return nullptr;
    }
    auto* extension = static_cast<ExtensionPlugin<Extension>*>(m_extensions[Extension::name].get());
    return extension->m_member.get();
  }

 public:
  static std::unique_ptr<RHIFactory>
  CreateInstance(const RendererType& rendererTyoe);

 protected:
  std::unordered_map<std::string_view, std::unique_ptr<ExtensionInterface>> m_extensions;
  std::unique_ptr<PipelineContext> m_pipelineContext = nullptr;
  std::unique_ptr<BufferContext> m_bufferContext = nullptr;
  std::unique_ptr<ImguiContext> m_imguiContext = nullptr;
};

}  // namespace Marbas
