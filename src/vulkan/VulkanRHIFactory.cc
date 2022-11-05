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

#include "VulkanRHIFactory.hpp"

#include <glog/logging.h>

#include "common.hpp"

namespace Marbas {

VulkanRHIFactory::VulkanRHIFactory() { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }

void
VulkanRHIFactory::Init(GLFWwindow* window, uint32_t width, uint32_t height) {
  // get glfw extensions
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // validation layers
  std::array<const char*, 1> layers = {"VK_LAYER_KHRONOS_validation"};

  // create vulkan instance
  vk::InstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.setEnabledExtensionCount(glfwExtensionCount);
  instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
  instanceCreateInfo.setEnabledLayerCount(layers.size());
  instanceCreateInfo.setPpEnabledLayerNames(layers.data());

  m_instance = vk::createInstance(instanceCreateInfo);

  // set surface KHR
  VkSurfaceKHR surface;
  auto instance = static_cast<VkInstance>(m_instance);
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  m_surface = static_cast<vk::SurfaceKHR>(surface);

  // create physiacal device
  auto physicalDevices = m_instance.enumeratePhysicalDevices();
  if (physicalDevices.size() == 1) {
    m_physicalDevice = physicalDevices[0];
  } else {
    for (auto device : physicalDevices) {
      m_physicalDevice = device;
      auto properties = device.getProperties();
      if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        m_physicalDevice = device;
        break;
      }
    }
  }
  auto properties = m_physicalDevice.getProperties();
  LOG(INFO) << FORMAT("find the physical device: {}, version: {}, api version: {}",
                      properties.deviceName, properties.driverVersion, properties.apiVersion);

  // fina queue family
  auto families = m_physicalDevice.getQueueFamilyProperties();
  uint32_t index = 0;
  for (const auto& family : families) {
    if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
      m_graphicsQueueFamilyIndex = index;
    }
    if (family.queueFlags | vk::QueueFlagBits::eTransfer) {
      m_transferQueueFamilyIndex = index;
    }
    if (m_physicalDevice.getSurfaceSupportKHR(index, m_surface)) {
      m_presentQueueFamilyIndex = index;
    }
    if (m_graphicsQueueFamilyIndex && m_presentQueueFamilyIndex && m_transferQueueFamilyIndex) {
      break;
    }
    index++;
  }

  /**
   * create logical device and queue
   */
  vk::DeviceCreateInfo deviceCreateInfo;
  std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::array<float, 1> priorities = {1.0};

  // create graphics queue create info
  vk::DeviceQueueCreateInfo queueCreateInfo;
  queueCreateInfo.setQueueFamilyIndex(m_graphicsQueueFamilyIndex);
  queueCreateInfo.setQueuePriorities(priorities);
  queueCreateInfo.setQueueCount(1);
  queueCreateInfos.push_back(queueCreateInfo);

  if (m_presentQueueFamilyIndex != m_graphicsQueueFamilyIndex) {
    queueCreateInfo.setQueueFamilyIndex(m_presentQueueFamilyIndex);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  if (m_transferQueueFamilyIndex != m_graphicsQueueFamilyIndex &&
      m_transferQueueFamilyIndex != m_presentQueueFamilyIndex) {
    queueCreateInfo.setQueueFamilyIndex(m_transferQueueFamilyIndex);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // create device and queue
  deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
  m_device = m_physicalDevice.createDevice(deviceCreateInfo);

  m_graphicsQueue = m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
  m_presentQueue = m_device.getQueue(m_presentQueueFamilyIndex, 0);
  m_transferQueue = m_device.getQueue(m_transferQueueFamilyIndex, 0);

  // find surface format and capabilities
  m_capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
  auto surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
  m_surfaceFormat = surfaceFormats[0];
  auto formatResultIter =
      std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(), [](const auto& surfaceFormat) {
        return surfaceFormat.format == vk::Format::eR8G8B8A8Unorm ||
               surfaceFormat.format == vk::Format::eB8G8R8A8Unorm ||
               surfaceFormat.format == vk::Format::eR8G8B8Unorm ||
               surfaceFormat.format == vk::Format::eB8G8R8Unorm;
      });
  if (formatResultIter != surfaceFormats.cend()) {
    m_surfaceFormat = *formatResultIter;
  }

  auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
  m_presentMode = vk::PresentModeKHR::eFifo;
  if (std::find(presentModes.cbegin(), presentModes.cend(), vk::PresentModeKHR::eMailbox) !=
      presentModes.cend()) {
    m_presentMode = vk::PresentModeKHR::eMailbox;
  }

  if (m_capabilities.currentExtent.width == 0xffffffff) {
    const auto& minImageExtent = m_capabilities.minImageExtent;
    const auto& maxImageExtent = m_capabilities.maxImageExtent;
    width = std::clamp<uint32_t>(width, minImageExtent.width, maxImageExtent.width);
    height = std::clamp<uint32_t>(height, minImageExtent.height, maxImageExtent.height);
  } else {
    width = m_capabilities.currentExtent.width;
    height = m_capabilities.currentExtent.height;
  }
  CreateSwapchain(width, height);
}

void
VulkanRHIFactory::CreateSwapchain(uint32_t width, uint32_t height) {
  int imageCount = 2;

  vk::SwapchainCreateInfoKHR swapChainCreateInfo;
  swapChainCreateInfo.setImageColorSpace(m_surfaceFormat.colorSpace);
  swapChainCreateInfo.setImageFormat(m_surfaceFormat.format);
  swapChainCreateInfo.setImageExtent(vk::Extent2D(width, height));
  swapChainCreateInfo.setMinImageCount(imageCount);
  swapChainCreateInfo.setPresentMode(m_presentMode);
  swapChainCreateInfo.setPreTransform(m_capabilities.currentTransform);
  swapChainCreateInfo.setClipped(true);
  swapChainCreateInfo.setSurface(m_surface);
  swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  swapChainCreateInfo.setImageArrayLayers(1);
  swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  if (m_graphicsQueueFamilyIndex == m_presentQueueFamilyIndex) {
    swapChainCreateInfo.setQueueFamilyIndices(m_graphicsQueueFamilyIndex);
    swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
  } else {
    std::array<uint32_t, 2> queueFamilIndices = {m_graphicsQueueFamilyIndex,
                                                 m_presentQueueFamilyIndex};
    swapChainCreateInfo.setQueueFamilyIndices(queueFamilIndices);
    swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
  }
  m_swapChainData.m_swapChain = m_device.createSwapchainKHR(swapChainCreateInfo);

  // create image and image view
  auto images = m_device.getSwapchainImagesKHR(m_swapChainData.m_swapChain);
  m_swapChainData.m_images = images;
  m_swapChainData.m_imageViews.clear();
  for (auto& image : images) {
    vk::ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.setImage(image);
    imageViewCreateInfo.setFormat(m_surfaceFormat.format);
    imageViewCreateInfo.setViewType(vk::ImageViewType::e2D);

    vk::ComponentMapping componentMapping;
    componentMapping.setR(vk::ComponentSwizzle::eR);
    componentMapping.setG(vk::ComponentSwizzle::eG);
    componentMapping.setB(vk::ComponentSwizzle::eB);
    componentMapping.setA(vk::ComponentSwizzle::eA);
    imageViewCreateInfo.setComponents(componentMapping);

    vk::ImageSubresourceRange range;
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    range.setBaseArrayLayer(0);
    range.setLayerCount(1);
    range.setBaseMipLevel(0);
    range.setLevelCount(1);
    imageViewCreateInfo.setSubresourceRange(range);

    m_swapChainData.m_imageViews.push_back(m_device.createImageView(imageViewCreateInfo));
  }
}

void
VulkanRHIFactory::GetSwapchain(Swapchain& swapchain) {}

}  // namespace Marbas
