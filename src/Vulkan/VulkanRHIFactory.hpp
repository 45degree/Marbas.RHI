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
  ~VulkanRHIFactory() override;

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

 public:
  static vk::Device
  GetVkDevice() {
    return m_device;
  }

  static vk::PhysicalDevice
  GetVkPhysicalDevice() {
    return m_physicalDevice;
  }

  static vk::Queue
  GetGraphicsQueue() {
    return m_graphicsQueue;
  }

 private:
  static vk::Device m_device;
  static vk::PhysicalDevice m_physicalDevice;
#ifndef NDEBUG
  static bool m_enableValidationLayer;
  static vk::DispatchLoaderDynamic m_dispatch;
  static vk::DebugUtilsMessengerEXT m_debugMessenger;
#endif

  static std::optional<uint32_t> m_graphicsQueueFamilyIndex;
  static std::optional<uint32_t> m_presentQueueFamilyIndex;
  static std::optional<uint32_t> m_transferQueueFamilyIndex;
  static std::optional<uint32_t> m_computeQueueFamilyIndex;

  static vk::Queue m_graphicsQueue;
  static vk::Queue m_presentQueue;
  static vk::Queue m_transferQueue;
  static vk::Queue m_computeQueue;

  static vk::Instance m_instance;
  static vk::SurfaceKHR m_surface;
  static vk::SurfaceFormatKHR m_surfaceFormat;
  static vk::SurfaceCapabilitiesKHR m_capabilities;
  static vk::PresentModeKHR m_presentMode;

  VulkanSwapchain m_swapChain = {};
};

}  // namespace Marbas
