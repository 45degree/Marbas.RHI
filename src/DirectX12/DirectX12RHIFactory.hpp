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

#include <d3d12.h>
#include <dxgi1_6.h>

#include "DirectX12Swapchain.hpp"
#include "RHIFactory.hpp"

namespace Marbas {

class DirectX12RHIFactory final : public RHIFactory {
 public:
  DirectX12RHIFactory();
  ~DirectX12RHIFactory();

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
  CreateGPUSemaphore() override;

  void
  DestroyGPUSemaphore(Semaphore* semaphore) override;

  // TODO: add supoort for offscreen
  // virtual void
  // OffscreenInit();

 private:
  uint32_t m_dxgiFactoryFlags = 0;
  IDXGIFactory6* m_dxgiFactory = nullptr;
  ID3D12Device* m_device = nullptr;
  IDXGIAdapter1* m_adapter = nullptr;

  // TODO: use command queue , graphics quque instead?
  ID3D12CommandQueue* m_commandQueue = nullptr;

#ifndef NDEBUG
  ID3D12Debug3* m_debugController = nullptr;
  ID3D12DebugDevice* m_debugDevice = nullptr;
#endif

  DirectX12Swapchain m_swapchain;
};

}  // namespace Marbas
