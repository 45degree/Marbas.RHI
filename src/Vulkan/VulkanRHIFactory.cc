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

#include "VulkanBufferContext.hpp"
#include "VulkanImage.hpp"
#include "VulkanImageView.hpp"
#include "VulkanImguiContext.hpp"
#include "VulkanPipelineContext.hpp"
#include "VulkanRHIFactory.hpp"
#include "VulkanSynchronic.hpp"
#include "common.hpp"

namespace Marbas {

VulkanRHIFactory::VulkanRHIFactory() { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
VulkanRHIFactory::~VulkanRHIFactory() = default;

void
VulkanRHIFactory::CreateInstance(GLFWwindow* glfwWindow) {
  // get glfw extensions
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // combine all extension needed to enable
  std::vector vulkanExtensions = {"VK_EXT_depth_clip_enable"};
  for (int i = 0; i < glfwExtensionCount; i++) {
    auto iter = std::find_if(vulkanExtensions.begin(), vulkanExtensions.end(), [&](const char* extension) -> bool {
      return std::strcmp(extension, *(glfwExtensions + i)) == 0;
    });
    if (iter == vulkanExtensions.end()) {
      vulkanExtensions.push_back(*(glfwExtensions + i));
    }
  }

  // validation layers
#ifndef NDEBUG
  std::array layers = {"VK_LAYER_KHRONOS_validation"};
#else
  std::array<const char*, 0> layers = {};
#endif

  // TODO: set error callback functions

  // create vulkan instance
  vk::InstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.setEnabledExtensionCount(glfwExtensionCount);
  instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
  instanceCreateInfo.setEnabledLayerCount(layers.size());
  instanceCreateInfo.setPpEnabledLayerNames(layers.data());

  m_instance = vk::createInstance(instanceCreateInfo);
}

void
VulkanRHIFactory::Init(GLFWwindow* window, uint32_t width, uint32_t height) {
  CreateInstance(window);

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
  LOG(INFO) << FORMAT("find the physical device: {}, version: {}, api version: {}", properties.deviceName,
                      properties.driverVersion, properties.apiVersion);

  // fina queue family
  auto families = m_physicalDevice.getQueueFamilyProperties();
  int index = 0;
  for (const auto& family : families) {
    if (family.queueFlags & vk::QueueFlagBits::eGraphics && !m_graphicsQueueFamilyIndex) {
      m_graphicsQueueFamilyIndex = index;
    }
    if (family.queueFlags & vk::QueueFlagBits::eTransfer && !m_transferQueueFamilyIndex) {
      m_transferQueueFamilyIndex = index;
    }
    if (family.queueFlags & vk::QueueFlagBits::eCompute && !m_computeQueueFamilyIndex) {
      m_computeQueueFamilyIndex = index;
    }
    if (m_physicalDevice.getSurfaceSupportKHR(index, m_surface) && !m_presentQueueFamilyIndex) {
      m_presentQueueFamilyIndex = index;
    }
    index++;
  }
  LOG_ASSERT(m_graphicsQueueFamilyIndex) << "can't find graphics queue family index";
  LOG_ASSERT(m_computeQueueFamilyIndex) << "can't find compute queue family index";
  LOG_ASSERT(m_transferQueueFamilyIndex) << "can't find transfer queue family index";
  LOG_ASSERT(m_presentQueueFamilyIndex) << "can't find present queue family index";

  /**
   * create logical device and queue
   */
  vk::DeviceCreateInfo deviceCreateInfo;
  std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::array<float, 1> priorities = {1.0};

  // set device features
  vk::PhysicalDeviceFeatures features;
  features.setDepthClamp(true);
  features.setGeometryShader(true);
  features.setDepthBiasClamp(true);
  deviceCreateInfo.setPEnabledFeatures(&features);

  // create graphics queue create info
  vk::DeviceQueueCreateInfo queueCreateInfo;
  queueCreateInfo.setQueueFamilyIndex(*m_graphicsQueueFamilyIndex);
  queueCreateInfo.setQueuePriorities(priorities);
  queueCreateInfo.setQueueCount(1);
  queueCreateInfos.push_back(queueCreateInfo);

  if (m_presentQueueFamilyIndex != m_graphicsQueueFamilyIndex) {
    queueCreateInfo.setQueueFamilyIndex(*m_presentQueueFamilyIndex);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  if (m_transferQueueFamilyIndex != m_graphicsQueueFamilyIndex &&
      m_transferQueueFamilyIndex != m_presentQueueFamilyIndex) {
    queueCreateInfo.setQueueFamilyIndex(*m_transferQueueFamilyIndex);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  if (m_computeQueueFamilyIndex != m_graphicsQueueFamilyIndex &&
      m_computeQueueFamilyIndex != m_presentQueueFamilyIndex &&
      m_computeQueueFamilyIndex != m_transferQueueFamilyIndex) {
    queueCreateInfo.setQueueFamilyIndex(*m_computeQueueFamilyIndex);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // create device and queue
  deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
  m_device = m_physicalDevice.createDevice(deviceCreateInfo);

  m_graphicsQueue = m_device.getQueue(*m_graphicsQueueFamilyIndex, 0);
  m_presentQueue = m_device.getQueue(*m_presentQueueFamilyIndex, 0);
  m_transferQueue = m_device.getQueue(*m_transferQueueFamilyIndex, 0);
  m_computeQueue = m_device.getQueue(*m_computeQueueFamilyIndex, 0);

  // find surface format and capabilities
  m_capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
  auto surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
  auto formatResultIter = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(), [](const auto& surfaceFormat) {
    return surfaceFormat.format == vk::Format::eR8G8B8A8Unorm || surfaceFormat.format == vk::Format::eB8G8R8A8Unorm;
  });
  if (formatResultIter == surfaceFormats.cend()) {
    constexpr std::string_view errMes = "can't find surface support RGBA or BGRA";
    LOG(ERROR) << errMes;
    throw std::runtime_error(errMes.data());
  }
  m_surfaceFormat = *formatResultIter;

  auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
  m_presentMode = vk::PresentModeKHR::eFifo;
  if (std::find(presentModes.cbegin(), presentModes.cend(), vk::PresentModeKHR::eMailbox) != presentModes.cend()) {
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

  m_pipelineContext = std::make_unique<VulkanPipelineContext>(m_device);
  m_bufferContext = std::make_unique<VulkanBufferContext>(VulkanBufferContextCreateInfo{
      .device = m_device,
      .physicalDevice = m_physicalDevice,
      .graphicsQueueIndex = *m_graphicsQueueFamilyIndex,
      .computeQueueIndex = *m_computeQueueFamilyIndex,
      .transfermQueueIndex = *m_transferQueueFamilyIndex,
      .graphicsQueue = m_graphicsQueue,
      .computeQueue = m_computeQueue,
      .transferQueue = m_transferQueue,
  });
  m_imguiContext = std::make_unique<VulkanImguiContext>(VulkanImguiCreateInfo{
      .glfwWindow = window,
      .instance = m_instance,
      .physicalDevice = m_physicalDevice,
      .graphicsQueueFamilyIndex = *m_graphicsQueueFamilyIndex,
      .graphicsQueue = m_graphicsQueue,
      .device = m_device,
      .swapchain = &m_swapChain,
      .width = width,
      .height = height,
  });
}

void
VulkanRHIFactory::CreateSwapchain(uint32_t width, uint32_t height) {
  int imageCount = 3;
  m_swapChain.width = width;
  m_swapChain.height = height;
  m_swapChain.surfaceFormat = m_surfaceFormat;
  m_swapChain.imageCount = imageCount;

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

  DLOG_ASSERT(m_graphicsQueueFamilyIndex);
  DLOG_ASSERT(m_computeQueueFamilyIndex);
  DLOG_ASSERT(m_transferQueueFamilyIndex);
  DLOG_ASSERT(m_presentQueueFamilyIndex);

  if (m_graphicsQueueFamilyIndex == m_presentQueueFamilyIndex) {
    auto graphicsQueueFamilyIndex = *m_graphicsQueueFamilyIndex;
    swapChainCreateInfo.setQueueFamilyIndices(graphicsQueueFamilyIndex);
    swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
  } else {
    std::array<uint32_t, 2> queueFamilIndices = {*m_graphicsQueueFamilyIndex, *m_presentQueueFamilyIndex};
    swapChainCreateInfo.setQueueFamilyIndices(queueFamilIndices);
    swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
  }
  m_swapChain.swapChain = m_device.createSwapchainKHR(swapChainCreateInfo);

  // create image and image view
  auto images = m_device.getSwapchainImagesKHR(m_swapChain.swapChain);
  m_swapChain.images.clear();
  m_swapChain.imageViews.clear();
  for (auto& image : images) {
    auto* vulkanImage = new VulkanImage();
    auto* vulkanImageView = new VulkanImageView();

    vulkanImage->vkImage = image;
    vulkanImage->usage = ImageUsageFlags::PRESENT | ImageUsageFlags::RENDER_TARGET;
    vulkanImage->aspect = vk::ImageAspectFlagBits::eColor;
    vulkanImage->mipMapLevel = 1;
    vulkanImage->arrayLayer = 1;
    vulkanImage->height = height;
    vulkanImage->width = width;
    if (m_surfaceFormat.format == vk::Format::eB8G8R8A8Unorm) {
      vulkanImage->format = ImageFormat::BGRA;
      m_swapChain.imageFormat = ImageFormat::BGRA;
    } else if (m_surfaceFormat.format == vk::Format::eR8G8B8A8Unorm) {
      vulkanImage->format = ImageFormat::RGBA;
      m_swapChain.imageFormat = ImageFormat::RGBA;
    }

    m_swapChain.images.push_back(vulkanImage);

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

    vulkanImageView->imageView = m_device.createImageView(imageViewCreateInfo);

    m_swapChain.imageViews.push_back(vulkanImageView);
  }
}

void
VulkanRHIFactory::DestroySwapchain() {
  for (auto* imageView : m_swapChain.imageViews) {
    auto* vulkanImageView = static_cast<VulkanImageView*>(imageView);
    m_device.destroyImageView(vulkanImageView->imageView);

    delete vulkanImageView;
  }
  m_device.destroySwapchainKHR(m_swapChain.swapChain);
}

void
VulkanRHIFactory::Quit() {
  m_device.waitIdle();
  DestroySwapchain();

  m_imguiContext = nullptr;
  m_bufferContext = nullptr;
  m_pipelineContext = nullptr;

  m_device.destroy();
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
}

Swapchain*
VulkanRHIFactory::GetSwapchain() {
  return &m_swapChain;
}

Swapchain*
VulkanRHIFactory::RecreateSwapchain(Swapchain* oldSwapchain, uint32_t width, uint32_t height) {
  m_device.waitIdle();
  DestroySwapchain();
  CreateSwapchain(width, height);
  return &m_swapChain;
}

int
VulkanRHIFactory::AcquireNextImage(Swapchain* swapchain, const Semaphore* semaphore) {
  auto* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain);
  auto currentSemaphore = static_cast<const VulkanSemaphore*>(semaphore)->semaphore;
  auto vkSwapchain = vulkanSwapchain->swapChain;

  auto nextImageResult = m_device.acquireNextImageKHR(vkSwapchain, UINT64_MAX, currentSemaphore);
  if (nextImageResult.result == vk::Result::eErrorOutOfDateKHR ||
      nextImageResult.result == vk::Result::eSuboptimalKHR) {
    return -1;
  }
  DLOG_ASSERT(nextImageResult.result == vk::Result::eSuccess);
  return static_cast<int>(nextImageResult.value);
}

int
VulkanRHIFactory::Present(Swapchain* swapchain, std::span<Semaphore*> waitSemaphores, uint32_t imageIndex) {
  auto* vulkanSwapchain = static_cast<VulkanSwapchain*>(swapchain);
  std::vector<vk::Semaphore> semaphores;
  std::transform(waitSemaphores.begin(), waitSemaphores.end(), std::back_inserter(semaphores),
                 [](const auto* semaphore) { return static_cast<const VulkanSemaphore*>(semaphore)->semaphore; });

  vk::PresentInfoKHR presentInfo;
  presentInfo.setWaitSemaphores(semaphores);
  presentInfo.setSwapchains(vulkanSwapchain->swapChain);
  presentInfo.setImageIndices(imageIndex);
  auto err2 = m_presentQueue.presentKHR(&presentInfo);

  if (err2 == vk::Result::eErrorOutOfDateKHR || err2 == vk::Result::eSuboptimalKHR) {
    return -1;
  }

  DLOG_ASSERT(err2 == vk::Result::eSuccess);
  return 1;
}

/**
 * synchronic
 */

Fence*
VulkanRHIFactory::CreateFence() {
  auto* fence = new VulkanFence();

  vk::FenceCreateInfo createInfo;
  fence->m_fence = m_device.createFence(createInfo);

  return fence;
}

void
VulkanRHIFactory::WaitForFence(Fence* fence) {
  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  auto result = m_device.waitForFences(vulkanFence->m_fence, true, std::numeric_limits<uint32_t>::max());
  return;
}

void
VulkanRHIFactory::ResetFence(Fence* fence) {
  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  m_device.resetFences(vulkanFence->m_fence);
}

void
VulkanRHIFactory::DestroyFence(Fence* fence) {
  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  m_device.destroyFence(vulkanFence->m_fence);

  delete vulkanFence;
}

Semaphore*
VulkanRHIFactory::CreateGPUSemaphore() {
  auto* semaphore = new VulkanSemaphore();

  vk::SemaphoreCreateInfo createInfo;
  semaphore->semaphore = m_device.createSemaphore(createInfo);

  return semaphore;
}

void
VulkanRHIFactory::DestroyGPUSemaphore(Semaphore* semaphore) {
  auto* vulkanSemaphore = static_cast<VulkanSemaphore*>(semaphore);
  m_device.destroySemaphore(vulkanSemaphore->semaphore);

  delete vulkanSemaphore;
}

}  // namespace Marbas
