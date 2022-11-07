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

namespace Marbas {

Buffer*
VulkanBufferContext::CreateBuffer(BufferType bufferType, void* data, uint32_t size, bool isStatic) {
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

  // TODO: create memory

  return vulkanBuffer;
}

void
VulkanBufferContext::UpdateBuffer(Buffer* buffer, void* data, uint32_t size, uintptr_t offset) {}

void
VulkanBufferContext::DestroyBuffer(Buffer* buffer) {}

}  // namespace Marbas
