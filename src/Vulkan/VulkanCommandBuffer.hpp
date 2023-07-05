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

#pragma once

#include <vulkan/vulkan.hpp>

#include "CommandBuffer.hpp"
#include "Synchronic.hpp"
#include "VulkanPipelineContext.hpp"

namespace Marbas {

class VulkanGraphicsCommandBuffer final : public GraphicsCommandBuffer {
  friend class VulkanBufferContext;

 public:
  VulkanGraphicsCommandBuffer(VulkanPipelineContext* ctx, vk::CommandBuffer cmdBuffer, vk::Queue queue)
      : GraphicsCommandBuffer(), m_pipelineCtx(ctx), m_commandBuffer(cmdBuffer), m_queue(queue) {}
  ~VulkanGraphicsCommandBuffer() override = default;

 public:
  void
  Begin() override;

  void
  End() override;

  void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) override;

  void
  BeginPipeline(uintptr_t pipeline, FrameBuffer* frameBuffer, const std::vector<ClearValue>& clearColor) override;

  void
  EndPipeline(uintptr_t pipeline) override;

  void
  BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t>& sets) override;

  void
  PushConstant(uintptr_t pipeline, const void* data, uint32_t size, uint32_t offset) override;

 public:
  void
  SetViewports(std::span<ViewportInfo> viewportInfos) override;

  void
  SetScissors(std::span<ScissorInfo> scissorInfos) override;

  void
  BindVertexBuffer(Buffer* buffer) override;

  void
  BindIndexBuffer(Buffer* buffer) override;

  void
  SetCullMode(CullMode cullMode) override;

  void
  Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

  void
  DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
              uint32_t firstInstance) override;

  void
  GenerateMipmap(Image* image, uint32_t mipLevel) override;

  void
  ClearColor(Image* image, const ClearValue& value, int baseLayer, int layerCount, int baseLevel,
             int levelCount) override;

  void
  CopyImage(Image* image, Image* dstImage, CopyImageRange& srcRange, CopyImageRange& dstRange, int x, int y,
            int z) override;

 private:
  vk::CommandBuffer m_commandBuffer;
  vk::CommandBuffer m_secondaryCommandBuffer;
  vk::Queue m_queue;
  VulkanPipelineContext* m_pipelineCtx;
};

class VulkanComputeCommandBuffer final : public ComputeCommandBuffer {
  friend class VulkanBufferContext;

 public:
  VulkanComputeCommandBuffer(VulkanPipelineContext* ctx, vk::CommandBuffer cmdBuffer, vk::Queue queue)
      : ComputeCommandBuffer(), m_pipelineCtx(ctx), m_commandBuffer(cmdBuffer), m_queue(queue) {}
  ~VulkanComputeCommandBuffer() override = default;

 public:
  void
  Begin() override;

  void
  End() override;

  void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) override;

  void
  BeginPipeline(uintptr_t pipeline) override;

  void
  EndPipeline(uintptr_t pipeline) override;

  void
  BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t>& sets) override;

  void
  Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

  void
  ClearColor(Image* image, const ClearValue& value, int baseLayer, int layerCount, int baseLevel,
             int levelCount) override;

 private:
  uint32_t m_queueFamily;
  vk::CommandBuffer m_commandBuffer;
  vk::Device m_device;
  vk::Queue m_queue;

  VulkanPipelineContext* m_pipelineCtx;
};

}  // namespace Marbas
