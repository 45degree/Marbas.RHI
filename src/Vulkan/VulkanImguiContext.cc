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
#include "VulkanImguiContext.hpp"

#include <glog/logging.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include "VulkanImage.hpp"
#include "VulkanSynchronic.hpp"
#include "common.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

static vk::RenderPass
CreateImguiRenderPass(const vk::Device& device, const vk::Format& surfaceFormat, bool clearEnable) {
  vk::AttachmentDescription attachment;
  attachment.format = surfaceFormat;
  attachment.samples = vk::SampleCountFlagBits::e1;
  attachment.loadOp = clearEnable ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare;
  attachment.storeOp = vk::AttachmentStoreOp::eStore;
  attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  attachment.initialLayout = vk::ImageLayout::eUndefined;
  attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference color_attachment;
  color_attachment.attachment = 0;
  color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass;
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment;
  vk::SubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.srcAccessMask = vk::AccessFlagBits::eNone;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo info;
  info.attachmentCount = 1;
  info.pAttachments = &attachment;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;

  return device.createRenderPass(info);
}

void
VulkanImguiContext::SetUpVulkanWindowData(bool clearEnable) {
  if (static_cast<VkDescriptorPool>(m_descriptorPool) != nullptr) {
    m_device.destroyDescriptorPool(m_descriptorPool);
  }
  // some uniforms in vulkan shader.
  const uint32_t maxSize = 1000;
  std::array<vk::DescriptorPoolSize, 11> poolSizes = {
      vk::DescriptorPoolSize{vk::DescriptorType::eSampler, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, maxSize},
      vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, maxSize},
  };
  vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
  descriptorPoolCreateInfo.setPoolSizes(poolSizes);
  descriptorPoolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
  descriptorPoolCreateInfo.setMaxSets(maxSize * poolSizes.size());
  m_descriptorPool = m_device.createDescriptorPool(descriptorPoolCreateInfo);

  for (auto* imageView : m_renderResultImageViews) {
    auto vulkanImageView = static_cast<VulkanImageView*>(imageView);
    auto vkImageView = vulkanImageView->vkImageView;

    // create frame buffer
    vk::FramebufferCreateInfo frameCreateInfo;
    frameCreateInfo.setAttachments(vkImageView);
    frameCreateInfo.setWidth(m_width);
    frameCreateInfo.setHeight(m_height);
    frameCreateInfo.setLayers(1);
    frameCreateInfo.setRenderPass(m_renderPass);
    m_framebuffers.push_back(m_device.createFramebuffer(frameCreateInfo));
  }
}

void
VulkanImguiContext::CreateWindowCommandBuffer() {
  // Create Command Buffers
  VkResult err;
  auto imageCount = m_renderResultImageViews.size();

  {
    vk::CommandPoolCreateInfo info;
    info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    info.setQueueFamilyIndex(m_graphicsQueueFamilyIndex);
    m_commandPool = m_device.createCommandPool(info);
  }

  {
    vk::CommandBufferAllocateInfo info = {};
    info.setCommandPool(m_commandPool);
    info.setLevel(vk::CommandBufferLevel::ePrimary);
    info.setCommandBufferCount(imageCount);
    m_commandBuffers = m_device.allocateCommandBuffers(info);
  }

  m_fences.clear();
  for (uint32_t i = 0; i < imageCount; i++) {
    vk::FenceCreateInfo info;
    info.setFlags(vk::FenceCreateFlagBits::eSignaled);
    auto fence = m_device.createFence(info);
    m_fences.push_back(fence);
  }
}

VulkanImguiContext::VulkanImguiContext(const VulkanImguiCreateInfo& createInfo)
    : ImguiContext(),
      m_glfwWindow(createInfo.glfwWindow),
      m_instance(createInfo.instance),
      m_physicalDevice(createInfo.physicalDevice),
      m_graphicsQueueFamilyIndex(createInfo.graphicsQueueFamilyIndex),
      m_graphicsQueue(createInfo.graphicsQueue),
      m_device(createInfo.device),
      m_width(createInfo.width),
      m_height(createInfo.height) {
  m_clearColor.color.float32[0] = 0;
  m_clearColor.color.float32[1] = 0;
  m_clearColor.color.float32[2] = 0;
  m_clearColor.color.float32[3] = 1;
}

VulkanImguiContext::~VulkanImguiContext() {
  std::for_each(m_framebuffers.begin(), m_framebuffers.end(), [&](const auto& frameBuffer) {
    m_device.destroyFramebuffer(frameBuffer);
    return;
  });

  std::for_each(m_commandBuffers.begin(), m_commandBuffers.end(), [&](const auto& commandBuffer) {
    m_device.freeCommandBuffers(m_commandPool, commandBuffer);
    return;
  });
  m_device.destroyCommandPool(m_commandPool);

  std::for_each(m_fences.begin(), m_fences.end(), [&](const auto& fence) {
    m_device.destroyFence(fence);
    return;
  });

  m_device.destroyRenderPass(m_renderPass);
  m_device.destroyDescriptorPool(m_descriptorPool);
}

void
VulkanImguiContext::Resize(uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
  if (width > 0 && height > 0) {
    ImGui_ImplVulkan_SetMinImageCount(2);

    m_device.waitIdle();
    auto imageCount = m_renderResultImageViews.size();
    for (int i = 0; i < imageCount; i++) {
      m_device.destroyFramebuffer(m_framebuffers[i]);
    }

    vk::FramebufferCreateInfo info;
    info.setRenderPass(m_renderPass);
    info.setWidth(width);
    info.setHeight(height);
    info.setLayers(1);

    for (uint32_t i = 0; i < imageCount; i++) {
      vk::ImageView imageView = static_cast<VulkanImageView*>(m_renderResultImageViews[i])->vkImageView;
      info.setAttachments(imageView);
      m_framebuffers[i] = m_device.createFramebuffer(info);
    }
  }
}

void
VulkanImguiContext::ClearUp() {
  m_device.waitIdle();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void
VulkanImguiContext::SetUpImguiBackend(GLFWwindow* windows) {
  // check the result image view
  vk::Format renderPassFormat = vk::Format::eR8G8B8A8Unorm;
  if (!m_renderResultImageViews.empty()) {
    renderPassFormat = static_cast<VulkanImageView*>(m_renderResultImageViews[0])->vkFormat;
    for (auto* imageView : m_renderResultImageViews) {
      auto vkFormat = static_cast<VulkanImageView*>(imageView)->vkFormat;
      if (vkFormat != renderPassFormat) {
        constexpr std::string_view errMsg = "the imgui render result image views's format are not equal";
        LOG(ERROR) << errMsg;
        throw std::runtime_error(errMsg.data());
      }
    }
  }

  m_renderPass = CreateImguiRenderPass(m_device, renderPassFormat, true);

  SetUpVulkanWindowData(true);
  CreateWindowCommandBuffer();

  ImGui_ImplGlfw_InitForVulkan(windows, true);
  ImGui_ImplVulkan_LoadFunctions(
      [](const char* function_name, void* vulkan_instance) {
        return static_cast<vk::Instance>(reinterpret_cast<VkInstance>(vulkan_instance)).getProcAddr(function_name);
      },
      m_instance);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = m_instance;
  init_info.PhysicalDevice = m_physicalDevice;
  init_info.Device = m_device;
  init_info.QueueFamily = m_graphicsQueueFamilyIndex;
  init_info.Queue = m_graphicsQueue;
  init_info.PipelineCache = nullptr;
  init_info.DescriptorPool = m_descriptorPool;
  init_info.Subpass = 0;
  init_info.MinImageCount = 2;
  init_info.ImageCount = m_renderResultImageViews.size();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&init_info, m_renderPass);

  // upload font

  // Use any command queue
  m_device.resetCommandPool(m_commandPool);

  vk::CommandBufferBeginInfo begin_info;
  begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  m_commandBuffers[0].begin(begin_info);

  ImGui_ImplVulkan_CreateFontsTexture(m_commandBuffers[0]);

  m_commandBuffers[0].end();

  vk::SubmitInfo end_info;
  end_info.setCommandBuffers(m_commandBuffers[0]);
  m_graphicsQueue.submit(end_info);

  m_device.waitIdle();

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void
VulkanImguiContext::RenderData(uint32_t imageIndex, const ImguiRenderDataInfo& renderInfo) {
  // VkResult err;
  ImDrawData* draw_data = ImGui::GetDrawData();

  // wait indefinitely instead of periodically checking
  auto err = m_device.waitForFences(m_fences[imageIndex], VK_TRUE, UINT64_MAX);
  DLOG_ASSERT(err == vk::Result::eSuccess);

  m_device.resetFences(m_fences[imageIndex]);
  m_commandBuffers[imageIndex].reset();

  {
    vk::CommandBufferBeginInfo info;
    info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    m_commandBuffers[imageIndex].begin(info);
  }

  {
    vk::RenderPassBeginInfo info;
    info.setRenderPass(m_renderPass);
    info.setFramebuffer(m_framebuffers[imageIndex]);
    info.setRenderArea(vk::Rect2D({}, vk::Extent2D(m_width, m_height)));
    info.setClearValues(m_clearColor);
    m_commandBuffers[imageIndex].beginRenderPass(info, vk::SubpassContents::eInline);
  }

  // Record dear imgui primitives into command buffer
  {
    auto vkCommandBuffer = static_cast<VkCommandBuffer>(m_commandBuffers[imageIndex]);
    ImGui_ImplVulkan_RenderDrawData(draw_data, vkCommandBuffer);
  }

  m_commandBuffers[imageIndex].endRenderPass();
  m_commandBuffers[imageIndex].end();

  {
    auto waitSem = static_cast<const VulkanSemaphore*>(renderInfo.waitSemaphore)->semaphore;
    auto signalSem = static_cast<const VulkanSemaphore*>(renderInfo.signalSemaphore)->semaphore;
    auto commandBuffer = static_cast<vk::CommandBuffer>(m_commandBuffers[imageIndex]);
    vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo info = {};
    info.setWaitSemaphores(waitSem);
    info.setWaitDstStageMask(wait_stage);
    info.setSignalSemaphores(signalSem);
    info.setCommandBuffers(commandBuffer);
    m_graphicsQueue.submit(info, m_fences[imageIndex]);
  }
}

ImTextureID
VulkanImguiContext::CreateImGuiImage(ImageView* imageView) {
  auto vulkanImage = static_cast<VulkanImageView*>(imageView);

  // create sampler
  vk::SamplerCreateInfo createInfo;
  createInfo.setMagFilter(vk::Filter::eLinear);
  createInfo.setMinFilter(vk::Filter::eLinear);
  createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
  createInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
  createInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
  createInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
  createInfo.setMinLod(-1000);
  createInfo.setMaxLod(-1000);
  createInfo.setMaxAnisotropy(1.0f);

  auto vkSampler = m_device.createSampler(createInfo);
  auto vkLayout = static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal);

  auto imTextureId = ImGui_ImplVulkan_AddTexture(vkSampler, vulkanImage->vkImageView, vkLayout);
  m_imguiTexture[imTextureId] = vkSampler;

  return imTextureId;
}

void
VulkanImguiContext::DestroyImGuiImage(ImTextureID imTextureId) {
  auto vkSampler = m_imguiTexture[imTextureId];
  m_device.destroySampler(vkSampler);
  ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(imTextureId));
}

}  // namespace Marbas
