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

#include "DirectX12Common.hpp"
#include "DirectX12Image.hpp"
#include "DirectX12Synchronic.hpp"
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
  ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
  ThrowIfFailed(dc->QueryInterface(IID_PPV_ARGS(&m_debugController)));
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

    // Check if the adapter supports Direct3D 12, and use that for the rest of the application
    if (SUCCEEDED(D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr))) {
      LOG(INFO) << FORMAT(L"{}", desc.Description);
      break;
    }
  }

  ThrowIfFailed(D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));

#ifndef NDEBUG
  ThrowIfFailed(m_device->QueryInterface(&m_debugDevice));
#endif

  // create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

  // create command allocator
  ID3D12CommandAllocator* commandAllocator;
  ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

  // create swapchain
  DXGI_SWAP_CHAIN_DESC1 sd;
  sd.BufferCount = 2;
  sd.Width = width;
  sd.Height = height;
  sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;

  IDXGISwapChain1* swapchain;
  ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue, nativeWindow, &sd, nullptr, nullptr, &swapchain));
  if (SUCCEEDED(swapchain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&swapchain))) {
    m_swapchain.swapchain = static_cast<IDXGISwapChain3*>(swapchain);
    m_swapchain.height = height;
    m_swapchain.width = width;
    m_swapchain.imageCount = 2;
    m_swapchain.imageFormat = ImageFormat::RGBA;

    for (int i = 0; i < m_swapchain.imageCount; i++) {
      auto* directx12Image = new DirectX12Image();
      m_swapchain.swapchain->GetBuffer(i, IID_PPV_ARGS(&directx12Image->resource));
      directx12Image->width = width;
      directx12Image->height = height;
      directx12Image->arrayLayer = 1;
      directx12Image->depth = 1;
      directx12Image->format = ImageFormat::RGBA;
      directx12Image->mipMapLevel = 1;
      directx12Image->sampleCount = SampleCount::BIT1;
      directx12Image->usage = ImageUsageFlags::PRESENT;
      m_swapchain.images.push_back(directx12Image);
    }
  }
  DLOG_ASSERT(m_swapchain.swapchain != nullptr);
}

Swapchain*
DirectX12RHIFactory::GetSwapchain() {
  return &m_swapchain;
}

int
DirectX12RHIFactory::AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) {
  auto* d3dSwapchain = static_cast<DirectX12Swapchain*>(swapchain);
  return static_cast<int>(d3dSwapchain->swapchain->GetCurrentBackBufferIndex());
}

int
DirectX12RHIFactory::Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) {
  return 0;
}

Fence*
DirectX12RHIFactory::CreateFence() {
  auto fence = new DirectX12Fence();
  ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence->dxFence)));
  return fence;
}

void
DirectX12RHIFactory::DestroyFence(Fence* fence) {
  auto directx12fence = static_cast<DirectX12Fence*>(fence);
  directx12fence->dxFence->Release();
  delete directx12fence;
}

Semaphore*
DirectX12RHIFactory::CreateGPUSemaphore() {
  auto semaphore = new DirectX12Semaphore();
  ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&semaphore->dxFence)));
  return semaphore;
}

void
DirectX12RHIFactory::DestroyGPUSemaphore(Semaphore* semaphore) {
  auto directx12Semaphore = static_cast<DirectX12Semaphore*>(semaphore);
  directx12Semaphore->dxFence->Release();
  delete directx12Semaphore;
}

}  // namespace Marbas
