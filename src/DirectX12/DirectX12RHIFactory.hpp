/**
 * Copyright 2022.11.3 45degree
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

#include <dxgi1_6.h>

#include "RHIFactory.hpp"

namespace Marbas {

class DirectX12RHIFactory final : public RHIFactory {
 public:
  void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) override;

 public:
  Swapchain*
  GetSwapchain() override;

  uint32_t
  AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) override;

  int
  Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) override;

 public:
  Fence*
  CreateFence() override;

  void
  DestroyFence(Fence* fence) override;

  Semaphore*
  CreateSemaphore() override;

  void
  DestroySemaphore(Semaphore* semaphore) override;

  // TODO: add supoort for offscreen
  // virtual void
  // OffscreenInit();

 private:
  uint32_t m_dxgiFactoryFlags = 0;
  IDXGIFactory2* m_dxgiFactory = nullptr;
};

}  // namespace Marbas
