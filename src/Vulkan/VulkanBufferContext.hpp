/**
 * Copyright 2022.10.31 45degree
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

#include "BufferContext.hpp"

namespace Marbas {

struct VulkanBufferContextCreateInfo final {
  vk::Device device;
  vk::PhysicalDevice physicalDevice;
  uint32_t graphicsQueueIndex;
  uint32_t computeQueueIndex;
  uint32_t transfermQueueIndex;
  vk::Queue graphicsQueue;
  vk::Queue computeQueue;
  vk::Queue transferQueue;
};

class VulkanBufferContext final : public BufferContext {
 public:
  explicit VulkanBufferContext(const VulkanBufferContextCreateInfo& createInfo)
      : BufferContext(),
        m_device(createInfo.device),
        m_physicalDevice(createInfo.physicalDevice),
        m_computeQueueIndex(createInfo.computeQueueIndex),
        m_graphicsQueueIndex(createInfo.graphicsQueueIndex),
        m_transfermQueueIndex(createInfo.transfermQueueIndex),
        m_computeQueue(createInfo.computeQueue),
        m_graphicsQueue(createInfo.graphicsQueue),
        m_transferQueue(createInfo.transferQueue) {}

 public:
  Buffer*
  CreateBuffer(BufferType bufferType, const void* data, uint32_t size, bool isStatic) override;

  void
  UpdateBuffer(Buffer* buffer, const void* data, uint32_t size, uintptr_t offset) override;

  void
  DestroyBuffer(Buffer* buffer) override;

 public:
  CommandPool*
  CreateCommandPool(CommandBufferUsage usage) override;

  void
  DestroyCommandPool(CommandPool* commandPool) override;

  CommandBuffer*
  CreateCommandBuffer(CommandPool* commandPool) override;

  void
  DestroyCommandBuffer(CommandPool* commandPool, CommandBuffer* commandBuffer) override;

 private:
  uint32_t
  FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

 private:
  uint32_t m_graphicsQueueIndex;
  uint32_t m_computeQueueIndex;
  uint32_t m_transfermQueueIndex;
  vk::Queue m_graphicsQueue;
  vk::Queue m_computeQueue;
  vk::Queue m_transferQueue;

  vk::Device m_device;
  vk::PhysicalDevice m_physicalDevice;
};

}  // namespace Marbas
