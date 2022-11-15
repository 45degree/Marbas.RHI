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

#include "VulkanBufferContext.hpp"

#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"

namespace Marbas {

Buffer*
VulkanBufferContext::CreateBuffer(BufferType bufferType, const void* data, uint32_t size, bool isStatic) {
  auto* vulkanBuffer = new VulkanBuffer();
  vulkanBuffer->size = size;
  vulkanBuffer->bufferType = bufferType;

  // create buffer
  vk::BufferCreateInfo createInfo;
  vk::BufferUsageFlags usage;
  switch (bufferType) {
    case BufferType::VERTEX_BUFFER:
      usage = vk::BufferUsageFlagBits::eVertexBuffer;
      break;
    case BufferType::INDEX_BUFFER:
      usage = vk::BufferUsageFlagBits::eIndexBuffer;
      break;
    case BufferType::UNIFORM_BUFFER:
      usage = vk::BufferUsageFlagBits::eUniformBuffer;
      break;
    case BufferType::STORAGE_BUFFER:
      usage = vk::BufferUsageFlagBits::eStorageBuffer;
      break;
  }
  createInfo.setSharingMode(vk::SharingMode::eExclusive);
  createInfo.setUsage(usage);
  createInfo.setSize(size);

  vulkanBuffer->vkBuffer = m_device.createBuffer(createInfo);

  auto memRequirement = m_device.getBufferMemoryRequirements(vulkanBuffer->vkBuffer);
  vk::MemoryPropertyFlags memPropertyFlag;
  memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostVisible;
  memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;
  vk::MemoryAllocateInfo allocateInfo;
  allocateInfo.setAllocationSize(memRequirement.size);
  allocateInfo.setMemoryTypeIndex(FindMemoryType(memRequirement.memoryTypeBits, memPropertyFlag));
  vulkanBuffer->vkMemory = m_device.allocateMemory(allocateInfo);
  m_device.bindBufferMemory(vulkanBuffer->vkBuffer, vulkanBuffer->vkMemory, 0);

  void* mapData = m_device.mapMemory(vulkanBuffer->vkMemory, 0, size);
  memcpy(mapData, data, size);
  m_device.unmapMemory(vulkanBuffer->vkMemory);

  // TODO: add stage buffer for non static vertex buffer

  return vulkanBuffer;
}

uint32_t
VulkanBufferContext::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  auto memProperties = m_physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

// TODO:
void
VulkanBufferContext::UpdateBuffer(Buffer* buffer, const void* data, uint32_t size, uintptr_t offset) {}

void
VulkanBufferContext::DestroyBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  m_device.destroyBuffer(vulkanBuffer->vkBuffer);
  m_device.freeMemory(vulkanBuffer->vkMemory);

  // TODO: free stage buffer and memory
}

CommandPool*
VulkanBufferContext::CreateCommandPool(CommandBufferUsage usage) {
  auto* vulkanCommandPool = new VulkanCommandPool();

  vk::CommandPoolCreateInfo vkCommandPoolCreateInfo;
  if (usage == CommandBufferUsage::GRAPHICS) {
    vkCommandPoolCreateInfo.setQueueFamilyIndex(m_graphicsQueueIndex);
  } else if (usage == CommandBufferUsage::COMPUTE) {
    vkCommandPoolCreateInfo.setQueueFamilyIndex(m_computeQueueIndex);
  }

  // TODO: how to set flags?
  vkCommandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

  vulkanCommandPool->vkCommandPool = m_device.createCommandPool(vkCommandPoolCreateInfo);
  vulkanCommandPool->usage = usage;

  return vulkanCommandPool;
}

void
VulkanBufferContext::DestroyCommandPool(CommandPool* commandPool) {
  auto* vulkanCommandPool = static_cast<VulkanCommandPool*>(commandPool);
  m_device.destroyCommandPool(vulkanCommandPool->vkCommandPool);

  delete vulkanCommandPool;
}

CommandBuffer*
VulkanBufferContext::CreateCommandBuffer(CommandPool* commandPool) {
  auto* vulkanCommandPool = static_cast<VulkanCommandPool*>(commandPool);

  vk::CommandBufferAllocateInfo vkCommandBufferAllocateInfo;
  vkCommandBufferAllocateInfo.setCommandBufferCount(1);
  vkCommandBufferAllocateInfo.setCommandPool(vulkanCommandPool->vkCommandPool);
  vkCommandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

  auto result = m_device.allocateCommandBuffers(vkCommandBufferAllocateInfo);

  uint32_t queueFamilyIndex = 0;
  vk::Queue queue;
  if (vulkanCommandPool->usage == CommandBufferUsage::GRAPHICS) {
    queueFamilyIndex = m_graphicsQueueIndex;
    queue = m_graphicsQueue;
  } else if (vulkanCommandPool->usage == CommandBufferUsage::COMPUTE) {
    queueFamilyIndex = m_computeQueueIndex;
    queue = m_computeQueue;
  }

  auto* vulkanCommandBuffer = new VulkanCommandBuffer(m_device, result[0], queueFamilyIndex, queue);

  return vulkanCommandBuffer;
}

void
VulkanBufferContext::DestroyCommandBuffer(CommandPool* commandPool, CommandBuffer* commandBuffer) {
  auto* vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer);
  auto* vulkanCommandPool = static_cast<VulkanCommandPool*>(commandPool);
  m_device.freeCommandBuffers(vulkanCommandPool->vkCommandPool, vulkanCommandBuffer->m_commandBuffer);

  delete vulkanCommandBuffer;
}

}  // namespace Marbas
