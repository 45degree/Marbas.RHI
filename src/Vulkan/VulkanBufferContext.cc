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

#include <glog/logging.h>

#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"

namespace Marbas {

VulkanBufferContext::VulkanBufferContext(const VulkanBufferContextCreateInfo& createInfo)
    : BufferContext(),
      m_device(createInfo.device),
      m_physicalDevice(createInfo.physicalDevice),
      m_computeQueueIndex(createInfo.computeQueueIndex),
      m_graphicsQueueIndex(createInfo.graphicsQueueIndex),
      m_transfermQueueIndex(createInfo.transfermQueueIndex),
      m_computeQueue(createInfo.computeQueue),
      m_graphicsQueue(createInfo.graphicsQueue),
      m_transferQueue(createInfo.transferQueue) {
  vk::CommandPoolCreateInfo vkCommandPoolCreateInfo;
  vkCommandPoolCreateInfo.setQueueFamilyIndex(m_transfermQueueIndex);
  vkCommandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  m_temporaryCommandPool = m_device.createCommandPool(vkCommandPoolCreateInfo);
}

VulkanBufferContext::~VulkanBufferContext() { m_device.destroyCommandPool(m_temporaryCommandPool); }

Buffer*
VulkanBufferContext::CreateBuffer(BufferType bufferType, const void* data, uint32_t size, bool isStatic) {
  auto* vulkanBuffer = new VulkanBuffer();
  vulkanBuffer->size = size;
  vulkanBuffer->bufferType = bufferType;

  if (!isStatic) {
    vk::MemoryPropertyFlags stagingMemPropertyFlag;
    vk::BufferUsageFlags stagingBufUsage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingMemPropertyFlag |= vk::MemoryPropertyFlagBits::eHostVisible;
    stagingMemPropertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;
    auto [stagingBuffer, stagingBufMemory] = CreateBuffer(size, stagingBufUsage, stagingMemPropertyFlag);
    vulkanBuffer->stageBuffer = stagingBuffer;
    vulkanBuffer->stageBufferMemory = stagingBufMemory;
  }

  // create buffer
  vk::BufferCreateInfo createInfo;
  vk::BufferUsageFlags usage;
  vk::MemoryPropertyFlags memPropertyFlag;
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

  if (!isStatic) {
    usage |= vk::BufferUsageFlagBits::eTransferDst;
    memPropertyFlag |= vk::MemoryPropertyFlagBits::eDeviceLocal;
  } else {
    memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostVisible;
    memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;
  }

  auto [buffer, bufferMemory] = CreateBuffer(size, usage, memPropertyFlag);
  vulkanBuffer->vkBuffer = buffer;
  vulkanBuffer->vkMemory = bufferMemory;

  if (isStatic) {
    void* mapData = m_device.mapMemory(vulkanBuffer->vkMemory, 0, size);
    memcpy(mapData, data, size);
    m_device.unmapMemory(vulkanBuffer->vkMemory);
  } else {
    DLOG_ASSERT(vulkanBuffer->stageBuffer && vulkanBuffer->stageBufferMemory);

    void* mapData = m_device.mapMemory(*(vulkanBuffer->stageBufferMemory), 0, size);
    memcpy(mapData, data, size);
    m_device.unmapMemory(*(vulkanBuffer->stageBufferMemory));
    CopyBuffer(*(vulkanBuffer->stageBuffer), vulkanBuffer->vkBuffer, vulkanBuffer->size);
  }

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

  if (vulkanBuffer->stageBuffer) {
    m_device.destroyBuffer(*(vulkanBuffer->stageBuffer));
  }

  if (vulkanBuffer->stageBufferMemory) {
    m_device.freeMemory(*(vulkanBuffer->stageBufferMemory));
  }

  delete vulkanBuffer;
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

std::tuple<vk::Buffer, vk::DeviceMemory>
VulkanBufferContext::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
  vk::BufferCreateInfo bufferInfo;
  bufferInfo.setSize(size);
  bufferInfo.setUsage(usage);
  bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

  auto buffer = m_device.createBuffer(bufferInfo);

  vk::MemoryRequirements memRequirements = m_device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  auto bufferMemory = m_device.allocateMemory(allocInfo);
  m_device.bindBufferMemory(buffer, bufferMemory, 0);

  return {buffer, bufferMemory};
}

void
VulkanBufferContext::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
  vk::CommandBufferAllocateInfo allocInfo{};
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandPool = m_temporaryCommandPool;
  allocInfo.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer;
  auto result = m_device.allocateCommandBuffers(allocInfo);
  commandBuffer = result[0];

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBuffer.begin(beginInfo);

  vk::BufferCopy copyRange;
  copyRange.setSize(size);
  copyRange.setSrcOffset(0);
  copyRange.setDstOffset(0);
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRange);
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(commandBuffer);
  m_transferQueue.submit(submitInfo);
  m_transferQueue.waitIdle();

  m_device.freeCommandBuffers(m_temporaryCommandPool, commandBuffer);
}

}  // namespace Marbas
