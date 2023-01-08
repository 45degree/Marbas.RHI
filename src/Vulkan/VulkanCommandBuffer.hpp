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

namespace Marbas {

struct VulkanCommandPool final : public CommandPool {
  vk::CommandPool vkCommandPool;
};

class VulkanCommandBuffer final : public CommandBuffer {
  friend class VulkanBufferContext;

 public:
  VulkanCommandBuffer(vk::Device device, vk::CommandBuffer buffer, uint32_t queueFamily, vk::Queue queue)
      : m_device(device), m_commandBuffer(buffer), m_queueFamily(queueFamily), m_queue(queue) {}
  virtual ~VulkanCommandBuffer() = default;

 public:
  void
  BindDescriptorSet(const Pipeline* pipeline, DescriptorSet* descriptors) override;

  void
  BindVertexBuffer(Buffer* buffer) override;

  void
  BindIndexBuffer(Buffer* buffer) override;

  void
  InsertBufferBarrier(const std::vector<BufferBarrier>& barrier) override;

  void
  InsertImageBarrier(const std::vector<ImageBarrier>& barriers) override;

 public:
  void
  Begin() override;

  void
  End() override;

  void
  BeginPipeline(Pipeline* pipeline, FrameBuffer* frameBuffer, const std::vector<ClearValue>& clearColors) override;

  void
  SetViewports(std::span<ViewportInfo> viewportInfos) override;

  void
  SetScissors(std::span<ScissorInfo> scissorInfos) override;

  void
  EndPipeline(Pipeline* pipeline) override;

  /**
   * @brief Draw primitives
   *
   * @param vertexCount the number of vertices to draw.
   * @param instanceCount the number of instances to draw.
   * @param firstVertex the index of the first vertex to draw.
   * @param firstInstance the instance ID of the first instance to draw.
   */
  void
  Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override {
    m_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  }

  /**
   * @brief Draw primitives with indexed vertices
   *
   * @param indexCount the number of vertices to draw.
   * @param instanceCount the number of instances to draw.
   * @param firstIndex the base index within the index buffer
   * @param vertexOffset A value added to each index before reading a vertex from the vertex buffer.
   * @param firstInstance the instance ID of the first instance to draw.
   */
  void
  DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
              uint32_t firstInstance) override {
    m_commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  }

 public:
  void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) override;

 private:
  uint32_t m_queueFamily;
  vk::CommandBuffer m_commandBuffer;
  vk::Device m_device;
  vk::Queue m_queue;
};

}  // namespace Marbas
