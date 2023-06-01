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
#include "Vulkan/VulkanFormat.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSynchronic.hpp"
#include "common.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

FORCE_INLINE vk::AccessFlags
ConvertToVulkanBufferAccess(uint32_t usage, const Buffer& buffer) {
  vk::AccessFlags flags;
  if (usage & BufferUsageFlags::TRANSFER_SRC) {
    flags |= vk::AccessFlagBits::eTransferRead;
  }
  if (usage & BufferUsageFlags::TRANSFER_DST) {
    flags |= vk::AccessFlagBits::eTransferWrite;
  }

  if (usage & BufferUsageFlags::READ) {
    switch (buffer.bufferType) {
      case BufferType::VERTEX_BUFFER:
        flags |= vk::AccessFlagBits::eVertexAttributeRead;
        break;
      case BufferType::INDEX_BUFFER:
        flags |= vk::AccessFlagBits::eIndexRead;
        break;
      case BufferType::UNIFORM_BUFFER:
        flags |= vk::AccessFlagBits::eUniformRead;
        break;
      case BufferType::STORAGE_BUFFER:
        flags |= vk::AccessFlagBits::eShaderRead;
        break;
      case BufferType::UNIFORM_TEXEL_BUFFER:
      case BufferType::STORAGE_TEXEL_BUFFER:
        break;
    }
  }

  if (usage & BufferUsageFlags::WRITE) {
    switch (buffer.bufferType) {
      case BufferType::VERTEX_BUFFER:
      case BufferType::INDEX_BUFFER:
      case BufferType::UNIFORM_BUFFER:
        flags |= vk::AccessFlagBits::eMemoryWrite;
        break;
      case BufferType::STORAGE_BUFFER:
        flags |= vk::AccessFlagBits::eShaderWrite;
        break;
      case BufferType::UNIFORM_TEXEL_BUFFER:
      case BufferType::STORAGE_TEXEL_BUFFER:
        break;
    }
  }

  return flags;
}

void
VulkanGraphicsCommandBuffer::BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t>& sets) {
  const auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));
  vk::PipelineLayout vkPipelineLayout;
  vk::PipelineBindPoint vkPipelineBindPoint;
  if (m_pipelineCtx->m_graphicsPipeline.find(vkPipeline) != m_pipelineCtx->m_graphicsPipeline.end()) {
    vkPipelineLayout = m_pipelineCtx->m_graphicsPipeline[vkPipeline].vkPipelineLayout;
    vkPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  } else {
    constexpr static std::string_view errMsg = "can't find pipeline in graphics pipeline cache";
    LOG(ERROR) << errMsg;
    throw std::runtime_error(errMsg.data());
  }

  std::vector<vk::DescriptorSet> vkDescriptorSets;
  vkDescriptorSets.reserve(sets.size());
  for (auto set : sets) {
    vkDescriptorSets.emplace_back(reinterpret_cast<VkDescriptorSet>(set));
  }

  m_commandBuffer.bindDescriptorSets(vkPipelineBindPoint, vkPipelineLayout, 0, vkDescriptorSets, nullptr);
}

void
VulkanGraphicsCommandBuffer::PushConstant(uintptr_t pipeline, const void* data, uint32_t size, uint32_t offset) {
  const auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));
  vk::PipelineLayout vkPipelineLayout;
  vk::PipelineBindPoint vkPipelineBindPoint;
  if (m_pipelineCtx->m_graphicsPipeline.find(vkPipeline) != m_pipelineCtx->m_graphicsPipeline.end()) {
    vkPipelineLayout = m_pipelineCtx->m_graphicsPipeline[vkPipeline].vkPipelineLayout;
    vkPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  } else {
    constexpr static std::string_view errMsg = "can't find pipeline in graphics pipeline cache";
    LOG(ERROR) << errMsg;
    throw std::runtime_error(errMsg.data());
  }

  m_commandBuffer.pushConstants(vkPipelineLayout, vk::ShaderStageFlagBits::eAll, offset, size, data);
}

void
VulkanGraphicsCommandBuffer::BindVertexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindVertexBuffers(0, vkBuffer, static_cast<vk::DeviceSize>(0));
}

void
VulkanGraphicsCommandBuffer::BindIndexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindIndexBuffer(vkBuffer, 0, vk::IndexType::eUint32);
}

void
VulkanGraphicsCommandBuffer::Begin() {
  vk::CommandBufferBeginInfo vkBeginInfo;
  vkBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  m_commandBuffer.begin(vkBeginInfo);
}

void
VulkanGraphicsCommandBuffer::End() {
  m_commandBuffer.end();
}

void
VulkanGraphicsCommandBuffer::BeginPipeline(uintptr_t pipeline, FrameBuffer* frameBuffer,
                                           const std::vector<ClearValue>& clearValues) {
  auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));
  auto vkRenderPass = m_pipelineCtx->m_graphicsPipeline[vkPipeline].vkRenderPass;
  auto* vulkanFrameBuffer = static_cast<VulkanFrameBuffer*>(frameBuffer);
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
  m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vkPipeline);
}

void
VulkanGraphicsCommandBuffer::EndPipeline(uintptr_t pipeline) {
  m_commandBuffer.endRenderPass();
}

void
VulkanGraphicsCommandBuffer::SetViewports(std::span<ViewportInfo> viewportInfos) {
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
VulkanGraphicsCommandBuffer::SetCullMode(CullMode cullMode) {
  m_commandBuffer.setCullMode(ConvertToVulkanCullMode(cullMode));
}

void
VulkanGraphicsCommandBuffer::SetScissors(std::span<ScissorInfo> scissorInfos) {
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
VulkanGraphicsCommandBuffer::Submit(std::span<Semaphore*> waitSemaphores, std::span<Semaphore*> signalSemaphores, Fence* fence) {
  std::vector<vk::Semaphore> vkWaitSemaphore, vkSignalSemaphore;
  for(auto* semaphore : waitSemaphores) {
    if(semaphore != nullptr) {
      vkWaitSemaphore.push_back(static_cast<VulkanSemaphore*>(semaphore)->semaphore);
    }
  }

  for(auto* semaphore : signalSemaphores) {
    if(semaphore != nullptr) {
      vkSignalSemaphore.push_back(static_cast<VulkanSemaphore*>(semaphore)->semaphore);
    }
  }

  vk::SubmitInfo vkSubmitInfo;
  vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eAllCommands;
  vkSubmitInfo.setWaitDstStageMask(waitDstStage);
  vkSubmitInfo.setSignalSemaphores(vkSignalSemaphore);
  vkSubmitInfo.setWaitSemaphores(vkWaitSemaphore);
  vkSubmitInfo.setCommandBuffers(m_commandBuffer);

  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  if (vulkanFence != nullptr) {
    m_queue.submit(vkSubmitInfo, vulkanFence->m_fence);
  } else {
    m_queue.submit(vkSubmitInfo);
  }
}

void
VulkanGraphicsCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
  m_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void
VulkanGraphicsCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
  m_commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void
VulkanGraphicsCommandBuffer::GenerateMipmap(Image* image, uint32_t mipLevels) {
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  auto vkImage = vulkanImage->vkImage;
  auto vkFormat = GetVkFormat(vulkanImage->format);
  auto vkPhysicalDevice = m_pipelineCtx->m_physicalDevice;
  auto defaultLayout = GetDefaultImageLayoutFromUsage(image->usage);

  // Check if image format supports linear blitting
  auto formatProperties = vkPhysicalDevice.getFormatProperties(vkFormat);
  if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
      throw std::runtime_error("texture image format does not support linear blitting!");
  }

  // transform the whole image from defulat layout to transform dst layout
  vk::PipelineStageFlags srcFlags, dstFlags;
  srcFlags = vk::PipelineStageFlagBits::eAllGraphics | vk::PipelineStageFlagBits::eTransfer;
  dstFlags = vk::PipelineStageFlagBits::eAllGraphics | vk::PipelineStageFlagBits::eTransfer;

  vk::ImageMemoryBarrier barrier;
  barrier.setImage(vulkanImage->vkImage);
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.oldLayout = defaultLayout;
  barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
  barrier.subresourceRange.aspectMask = vulkanImage->vkAspect;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = image->arrayLayer;
  barrier.subresourceRange.levelCount = image->mipMapLevel;
  barrier.srcAccessMask = vk::AccessFlagBits::eNone;
  barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
  m_commandBuffer.pipelineBarrier(srcFlags, dstFlags, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

  barrier.subresourceRange.levelCount = 1;
  int mipWidth = static_cast<int>(image->width);
  int mipHeight = static_cast<int>(image->height);
  int mipDepth = static_cast<int>(image->depth);
  for (uint32_t i = 1; i < mipLevels; i++) {
    // convert the last level of image to the transfer src layout
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

    m_commandBuffer.pipelineBarrier(srcFlags, dstFlags, vk::DependencyFlagBits::eByRegion, nullptr, nullptr,barrier);

    // blit the last level of image to the current level
    vk::ImageBlit blit;
    blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, mipDepth);
    blit.srcSubresource.aspectMask = vulkanImage->vkAspect;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = image->arrayLayer;

    blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.dstOffsets[1] = vk::Offset3D( mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 );
    blit.dstSubresource.aspectMask = vulkanImage->vkAspect;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = image->arrayLayer;

    vk::ImageLayout srcLayout, dstLayout;
    srcLayout = vk::ImageLayout::eTransferSrcOptimal;
    dstLayout = vk::ImageLayout::eTransferDstOptimal;
    m_commandBuffer.blitImage(vkImage, srcLayout, vkImage, dstLayout, blit, vk::Filter::eLinear);

    // convert the last level of image to the default Layout and enable shader to read
    barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier.newLayout = defaultLayout;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    m_commandBuffer.pipelineBarrier(srcFlags, dstFlags, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
    if (mipDepth > 1) mipDepth /= 2;
  }

  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
  barrier.newLayout = defaultLayout;
  barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

  srcFlags = vk::PipelineStageFlagBits::eAllCommands;
  dstFlags = vk::PipelineStageFlagBits::eAllCommands;
  m_commandBuffer.pipelineBarrier(srcFlags, dstFlags, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);
}

void
VulkanGraphicsCommandBuffer::ClearColorImage(Image* image, const ClearValue& value, int baseLayer, int layerCount, int baseLevel, int levelCount) {

  auto* vulkanImage = static_cast<VulkanImage*>(image);
  auto vkImage=  vulkanImage->vkImage;
  auto layout = GetDefaultImageLayoutFromUsage(vulkanImage->usage);

  vk::ImageSubresourceRange range;
  range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  range.setBaseArrayLayer(baseLayer);
  range.setLayerCount(layerCount);
  range.setBaseMipLevel(baseLevel);
  range.setLevelCount(levelCount);

  auto* clearColor = std::get_if<0>(&value.clearValue);
  LOG_IF(WARNING, clearColor == nullptr) << "the clear value of command(Clear Color Image) is not a color";
  vk::ClearColorValue vkClearValue(*clearColor);

  m_commandBuffer.clearColorImage(vkImage, layout, vkClearValue, range);

}

/**
 * compute command buffer
 */
void
VulkanComputeCommandBuffer::Begin() {
  vk::CommandBufferBeginInfo vkBeginInfo;
  vkBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  m_commandBuffer.begin(vkBeginInfo);
}

void
VulkanComputeCommandBuffer::End() {
  m_commandBuffer.end();
}

void
VulkanComputeCommandBuffer::BeginPipeline(uintptr_t pipeline) {
  auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));
  m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, vkPipeline);
}

void
VulkanComputeCommandBuffer::EndPipeline(uintptr_t pipeline) {}

void
VulkanComputeCommandBuffer::Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) {
  std::vector<vk::Semaphore> vkWaitSemaphore, vkSignalSemaphore;
  std::transform(waitSemaphore.begin(), waitSemaphore.end(), std::back_inserter(vkWaitSemaphore),
                 [](auto* semaphore) ->vk::Semaphore {
                   if(semaphore == nullptr) return nullptr;
                   return static_cast<VulkanSemaphore*>(semaphore)->semaphore;
                 });
  std::transform(signalSemaphore.begin(), signalSemaphore.end(), std::back_inserter(vkSignalSemaphore),
                 [](auto* semaphore) ->vk::Semaphore { 
                   if(semaphore == nullptr) return nullptr;
                   return static_cast<VulkanSemaphore*>(semaphore)->semaphore; 
                 });

  vk::SubmitInfo vkSubmitInfo;
  vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eAllCommands;
  vkSubmitInfo.setWaitDstStageMask(waitDstStage);
  vkSubmitInfo.setSignalSemaphores(vkSignalSemaphore);
  vkSubmitInfo.setWaitSemaphores(vkWaitSemaphore);
  vkSubmitInfo.setCommandBuffers(m_commandBuffer);

  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  if (vulkanFence != nullptr) {
    m_queue.submit(vkSubmitInfo, vulkanFence->m_fence);
  } else {
    m_queue.submit(vkSubmitInfo);
  }
}

void
VulkanComputeCommandBuffer::BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t> &sets) {
  auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));

  vk::PipelineLayout vkPipelineLayout;
  vk::PipelineBindPoint vkPipelineBindPoint;
  if (m_pipelineCtx->m_computePipeline.find(vkPipeline) != m_pipelineCtx->m_computePipeline.end()) {
    vkPipelineLayout = m_pipelineCtx->m_computePipeline[vkPipeline].vkPipelineLayout;
    vkPipelineBindPoint = vk::PipelineBindPoint::eCompute;
  } else {
    constexpr static std::string_view errMsg = "can't find pipeline in compute pipeline cache";
    LOG(ERROR) << errMsg;
    throw std::runtime_error(errMsg.data());
  }

  std::vector<vk::DescriptorSet> vkDescriptorSets;
  vkDescriptorSets.reserve(sets.size());
  for (auto set : sets) {
    vkDescriptorSets.emplace_back(reinterpret_cast<VkDescriptorSet>(set));
  }

  m_commandBuffer.bindDescriptorSets(vkPipelineBindPoint, vkPipelineLayout, 0, vkDescriptorSets, nullptr);
}

void
VulkanComputeCommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
  m_commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
}


void
VulkanComputeCommandBuffer::ClearColorImage(Image* image, const ClearValue& value, int baseLayer, int layerCount, int baseLevel,
                  int levelCount) {
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  auto vkImage=  vulkanImage->vkImage;
  auto layout = GetDefaultImageLayoutFromUsage(vulkanImage->usage);

  vk::ImageSubresourceRange range;
  range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  range.setBaseArrayLayer(baseLayer);
  range.setLayerCount(layerCount);
  range.setBaseMipLevel(baseLevel);
  range.setLevelCount(levelCount);

  auto* clearColor = std::get_if<0>(&value.clearValue);
  LOG_IF(WARNING, clearColor == nullptr) << "the clear value of command(Clear Color Image) is not a color";
  vk::ClearColorValue vkClearValue(*clearColor);

  m_commandBuffer.clearColorImage(vkImage, layout, vkClearValue, range);
}

}  // namespace Marbas
