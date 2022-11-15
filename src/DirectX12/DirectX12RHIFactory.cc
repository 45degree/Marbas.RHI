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

#include "DirectX12RHIFactory.hpp"

#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

#include <iostream>

#include "common.hpp"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#ifndef NDEBUG
#include <dxgidebug.h>
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

namespace Marbas {

DirectX12RHIFactory::DirectX12RHIFactory() { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }

DirectX12RHIFactory::~DirectX12RHIFactory() { m_adapter->Release(); }

void
DirectX12RHIFactory::Init(GLFWwindow* window, uint32_t width, uint32_t height) {
  auto nativeWindow = glfwGetWin32Window(window);

#ifndef NDEBUG
  ID3D12Debug* dc = nullptr;
  D3D12GetDebugInterface(IID_PPV_ARGS(&dc));
  dc->QueryInterface(IID_PPV_ARGS(&m_debugController));
  m_debugController->EnableDebugLayer();
  m_debugController->SetEnableGPUBasedValidation(true);
  m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

  dc->Release();
  dc = nullptr;
#endif

  auto result = CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));

  for (int i = 0; m_dxgiFactory->EnumAdapters1(i, &m_adapter) != DXGI_ERROR_NOT_FOUND; i++) {
    DXGI_ADAPTER_DESC1 desc;
    m_adapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }

    if (SUCCEEDED(D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr))) {
      auto str = FORMAT(L"{}", desc.Description);
      std::wprintf(L"%s", str.data());
      break;
    }
  }

  D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
  DLOG_ASSERT(m_device != nullptr);

#ifndef NDEBUG
  m_device->QueryInterface(&m_debugDevice);
  DLOG_ASSERT(m_debugDevice != nullptr);
#endif

  // create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
  DLOG_ASSERT(m_commandQueue != nullptr);

  // create swapchain
  DXGI_SWAP_CHAIN_DESC sd;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = 2;
  sd.OutputWindow = nativeWindow;
  sd.Windowed = true;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  IDXGISwapChain* swapchain;
  m_dxgiFactory->CreateSwapChain(m_commandQueue, &sd, &swapchain);
  swapchain->QueryInterface(IID_PPV_ARGS(&m_swapchain.swapchain));
  DLOG_ASSERT(m_swapchain.swapchain != nullptr);
}

Swapchain*
DirectX12RHIFactory::GetSwapchain() {
  return &m_swapchain;
}

int
DirectX12RHIFactory::AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) {
  auto* d3dSwapchain = static_cast<DirectX12Swapchain*>(swapchain);

  return 0;
}

int
DirectX12RHIFactory::Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) {
  return 0;
}

Fence*
DirectX12RHIFactory::CreateFence() {
  return nullptr;
}

void
DirectX12RHIFactory::DestroyFence(Fence* fence) {}

Semaphore*
DirectX12RHIFactory::CreateGPUSemaphore() {
  return nullptr;
}

void
DirectX12RHIFactory::DestroyGPUSemaphore(Semaphore* semaphore) {}

}  // namespace Marbas
