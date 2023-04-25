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

}  // namespace Marbas
