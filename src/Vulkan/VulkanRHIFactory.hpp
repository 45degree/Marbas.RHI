/**
 * Copyright 2022.11.5 45degree
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

#include <optional>
#include <vulkan/vulkan.hpp>

#include "RHIFactory.hpp"
#include "VulkanSwapChain.hpp"

namespace Marbas {

class VulkanRHIFactory final : public RHIFactory {
 public:
  VulkanRHIFactory();
  virtual ~VulkanRHIFactory();

 public:
  std::vector<SampleCount>
  FindSupportSampleCount() override {
    return {};
  }

 public:
  Swapchain*
  GetSwapchain() override;

  Swapchain*
  RecreateSwapchain(Swapchain* oldSwapchain, uint32_t width, uint32_t height) override;

  int
  AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) override;

  int
  Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) override;

  void
  WaitIdle() override {
    m_device.waitIdle();
  }

 public:
  Fence*
  CreateFence() override;

  void
  WaitForFence(Fence* fence) override;

  void
  ResetFence(Fence* fence) override;

  void
  DestroyFence(Fence* fence) override;

  Semaphore*
  CreateGPUSemaphore() override;

  void
  DestroyGPUSemaphore(Semaphore* semaphore) override;

  void
  Init(GLFWwindow* window, uint32_t width, uint32_t height) override;

  void
  Quit() override;

 private:
  void
  CreateInstance(GLFWwindow* glfwWindow);

  void
  CreateSwapchain(uint32_t width, uint32_t height);

  void
  DestroySwapchain();

 private:
  vk::Device m_device;
  vk::PhysicalDevice m_physicalDevice;
#ifndef NDEBUG
  vk::DispatchLoaderDynamic m_dispatch;
  vk::DebugUtilsMessengerEXT m_debugMessenger;
#endif

  std::optional<uint32_t> m_graphicsQueueFamilyIndex = std::nullopt;
  std::optional<uint32_t> m_presentQueueFamilyIndex = std::nullopt;
  std::optional<uint32_t> m_transferQueueFamilyIndex = std::nullopt;
  std::optional<uint32_t> m_computeQueueFamilyIndex = std::nullopt;

  vk::Queue m_graphicsQueue;
  vk::Queue m_presentQueue;
  vk::Queue m_transferQueue;
  vk::Queue m_computeQueue;

  vk::Instance m_instance;
  vk::SurfaceKHR m_surface;
  vk::SurfaceFormatKHR m_surfaceFormat;
  vk::SurfaceCapabilitiesKHR m_capabilities;
  vk::PresentModeKHR m_presentMode;

  VulkanSwapchain m_swapChain = {};
};

}  // namespace Marbas
