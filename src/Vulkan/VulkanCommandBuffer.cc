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

#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "Buffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDescriptor.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSynchronic.hpp"
#include "common.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

void
VulkanCommandBuffer::InsertBufferBarrier(const std::vector<BufferBarrier>& barriers) {
  std::vector<vk::BufferMemoryBarrier> bufferMemoryBarrier;
  for (const auto& bufferBarrier : barriers) {
    vk::BufferMemoryBarrier vulkanBufferBarrier;
    const auto* buffer = bufferBarrier.buffer;
    auto vkBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer;
    const auto& srcUsage = bufferBarrier.waitUsage;
    const auto& dstUsage = bufferBarrier.dstUsage;
    const auto& size = buffer->size;
    vulkanBufferBarrier.setBuffer(vkBuffer);
    vulkanBufferBarrier.setOffset(0);
    vulkanBufferBarrier.setSize(size);
    vulkanBufferBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setSrcAccessMask(ConvertToVulkanBufferAccess(srcUsage, *buffer));
    vulkanBufferBarrier.setDstAccessMask(ConvertToVulkanBufferAccess(dstUsage, *buffer));

    bufferMemoryBarrier.push_back(vulkanBufferBarrier);
  }

  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
                                  vk::DependencyFlagBits::eByRegion, nullptr, bufferMemoryBarrier, nullptr);
}

void
VulkanCommandBuffer::InsertImageBarrier(const std::vector<ImageBarrier>& barriers) {
  std::vector<vk::ImageMemoryBarrier> imageMemoryBarrier;

  for (const auto& imageBarrier : barriers) {
    vk::ImageMemoryBarrier vulkanImageBarrier;
    const auto* vulkanImage = static_cast<const VulkanImage*>(imageBarrier.image);
    const auto& srcUsage = imageBarrier.waitUsage;
    const auto& dstUsage = imageBarrier.dstUsage;
    vulkanImageBarrier.setImage(vulkanImage->vkImage);
    vulkanImageBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcAccessMask(ConvertToVulkanImageAccess(srcUsage));
    vulkanImageBarrier.setDstAccessMask(ConvertToVulkanImageAccess(dstUsage));
    vulkanImageBarrier.setOldLayout(vk::ImageLayout::eGeneral);
    vulkanImageBarrier.setNewLayout(vk::ImageLayout::eGeneral);

    vk::ImageSubresourceRange range;
    range.setAspectMask(vulkanImage->vkAspect);
    range.setBaseArrayLayer(0);
    range.setBaseMipLevel(0);
    range.setLayerCount(vulkanImage->arrayLayer);
    range.setLevelCount(vulkanImage->mipMapLevel);
    vulkanImageBarrier.setSubresourceRange(range);

    imageMemoryBarrier.push_back(vulkanImageBarrier);
  }
  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

void
VulkanCommandBuffer::TransformImageState(Image* image, ImageState srcState, ImageState dstState) {
  vk::ImageMemoryBarrier imageMemoryBarrier;
  const auto* vulkanImage = static_cast<VulkanImage*>(image);

  imageMemoryBarrier.setImage(vulkanImage->vkImage);
  imageMemoryBarrier.setDstQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setOldLayout(ConvertToVulkanImageLayout(srcState));
  imageMemoryBarrier.setNewLayout(ConvertToVulkanImageLayout(dstState));

  vk::ImageSubresourceRange range;
  range.setAspectMask(vulkanImage->vkAspect);
  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLayerCount(vulkanImage->arrayLayer);
  range.setLevelCount(vulkanImage->mipMapLevel);
  imageMemoryBarrier.setSubresourceRange(range);

  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

void
VulkanCommandBuffer::BindDescriptorSet(const Pipeline* pipeline, DescriptorSet* descriptors) {
  const auto* vulkanPipeline = static_cast<const VulkanPipeline*>(pipeline);
  auto* vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptors);
  vk::PipelineBindPoint vkPipelineBindPoint;
  if (vulkanPipeline->pipelineType == PipelineType::GRAPHICS) {
    vkPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  } else if (vulkanPipeline->pipelineType == PipelineType::COMPUTE) {
    vkPipelineBindPoint = vk::PipelineBindPoint::eCompute;
  }

  std::vector<vk::DescriptorSet> vkDescriptorSets;
  vkDescriptorSets.push_back(vulkanDescriptorSet->uniformBufferSet);
  vkDescriptorSets.push_back(vulkanDescriptorSet->sampledImageSet);

  m_commandBuffer.bindDescriptorSets(vkPipelineBindPoint, vulkanPipeline->vkPipelineLayout, 0, vkDescriptorSets,
                                     nullptr);
}

void
VulkanCommandBuffer::BindVertexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindVertexBuffers(0, vkBuffer, static_cast<vk::DeviceSize>(0));
}

void
VulkanCommandBuffer::BindIndexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindIndexBuffer(vkBuffer, 0, vk::IndexType::eUint32);
}

void
VulkanCommandBuffer::Begin() {
  vk::CommandBufferBeginInfo vkBeginInfo;
  vkBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  m_commandBuffer.begin(vkBeginInfo);
}

void
VulkanCommandBuffer::End() {
  m_commandBuffer.end();
}

void
VulkanCommandBuffer::BeginPipeline(Pipeline* pipeline, FrameBuffer* frameBuffer,
                                   const std::vector<ClearValue>& clearValues) {
  auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
  auto* vulkanFrameBuffer = static_cast<VulkanFrameBuffer*>(frameBuffer);
  auto vkRenderPass = vulkanPipeline->vkRenderPass;
  auto vkPipeline = vulkanPipeline->vkPipeline;
  auto vkFramebuffer = vulkanFrameBuffer->vkFrameBuffer;
  const auto& height = frameBuffer->height;
  const auto& width = frameBuffer->width;

  vk::RenderPassBeginInfo vkRenderPassBeginInfo;
  std::vector<vk::ClearValue> vkClearValues;

  for (const auto& clearValue : clearValues) {
    // clang-format off
    std::visit([&](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      if constexpr (std::is_same_v<T, std::array<float, 4>>) {
        vkClearValues.emplace_back(vk::ClearColorValue(value));
      } else if constexpr (std::is_same_v<T, std::array<float, 2>>) {
        vkClearValues.emplace_back(vk::ClearDepthStencilValue(value[0], value[1]));
      }
    },clearValue.clearValue);
    // lcnag-format on
  }

  vkRenderPassBeginInfo.setRenderPass(vkRenderPass);
  vkRenderPassBeginInfo.setFramebuffer(vkFramebuffer);
  vkRenderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)));
  vkRenderPassBeginInfo.setClearValues(vkClearValues);

  m_commandBuffer.beginRenderPass(vkRenderPassBeginInfo, vk::SubpassContents::eInline);

  if (vulkanPipeline->pipelineType == PipelineType::GRAPHICS) {
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vkPipeline);
  } else if (vulkanPipeline->pipelineType == PipelineType::COMPUTE) {
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, vkPipeline);
  }
}

void
VulkanCommandBuffer::EndPipeline(Pipeline* pipeline) {
  m_commandBuffer.endRenderPass();
}

void
VulkanCommandBuffer::SetViewports(std::span<ViewportInfo> viewportInfos) {
  std::vector<vk::Viewport> vkViewports;
  std::transform(viewportInfos.begin(), viewportInfos.end(), std::back_inserter(vkViewports), [](auto& viewportInfo) {
    vk::Viewport vkViewport;
    vkViewport.setHeight(viewportInfo.height);
    vkViewport.setWidth(viewportInfo.width);
    vkViewport.setX(viewportInfo.x);
    vkViewport.setY(viewportInfo.y);
    vkViewport.setMinDepth(viewportInfo.minDepth);
    vkViewport.setMaxDepth(viewportInfo.maxDepth);
    return vkViewport;
  });

  m_commandBuffer.setViewport(0, vkViewports);
}

void
VulkanCommandBuffer::SetScissors(std::span<ScissorInfo> scissorInfos) {
  std::vector<vk::Rect2D> vkScissors;
  std::transform(scissorInfos.begin(), scissorInfos.end(), std::back_inserter(vkScissors), [](auto& scissorInfo) {
    vk::Rect2D scissor;
    scissor.setOffset(vk::Offset2D(scissorInfo.x, scissorInfo.y));
    scissor.setExtent(vk::Extent2D(scissorInfo.width, scissorInfo.height));
    return scissor;
  });
  m_commandBuffer.setScissor(0, vkScissors);
}

void
VulkanCommandBuffer::Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) {
  std::vector<vk::Semaphore> vkWaitSemaphore, vkSignalSemaphore;
  std::transform(waitSemaphore.begin(), waitSemaphore.end(), std::back_inserter(vkWaitSemaphore),
                 [](auto* semaphore) { return static_cast<VulkanSemaphore*>(semaphore)->semaphore; });
  std::transform(signalSemaphore.begin(), signalSemaphore.end(), std::back_inserter(vkSignalSemaphore),
                 [](auto* semaphore) { return static_cast<VulkanSemaphore*>(semaphore)->semaphore; });

  vk::SubmitInfo vkSubmitInfo;
  vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eAllCommands;
  vkSubmitInfo.setSignalSemaphores(vkSignalSemaphore);
  vkSubmitInfo.setWaitSemaphores(vkWaitSemaphore);
  vkSubmitInfo.setCommandBuffers(m_commandBuffer);
  vkSubmitInfo.setWaitDstStageMask(waitDstStage);

  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  if (vulkanFence != nullptr) {
    m_queue.submit(vkSubmitInfo, vulkanFence->m_fence);
  } else {
    m_queue.submit(vkSubmitInfo);
  }
}

}  // namespace Marbas
