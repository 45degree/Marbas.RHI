#include "VulkanCommandBuffer.hpp"

#include "vulkanBuffer.hpp"

namespace Marbas {

void
VulkanCommandBuffer::InsertResourceBarrier(ResourceBarrier &barrier) {
  auto size = barrier.bufferBarrier[0].buffer.size;
  auto buffer = barrier.bufferBarrier[0].buffer.vulkanData->buffer;

  vk::BufferMemoryBarrier bufferBarrier;
  bufferBarrier.setBuffer(buffer);
  bufferBarrier.setOffset(0);
  bufferBarrier.setSize(size);
  bufferBarrier.setSrcQueueFamilyIndex(m_queueFamily);
  bufferBarrier.setDstQueueFamilyIndex(m_queueFamily);

  // TODO: use buffer type and barrier usage to set access flag bits
}

}  // namespace Marbas
