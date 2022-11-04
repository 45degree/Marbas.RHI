#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "Buffer.hpp"
#include "common.hpp"
#include "vulkanBuffer.hpp"

namespace Marbas {

FORCE_INLINE static vk::AccessFlags
ConvertToVulkanAccessFlags(const ResourceUsage& usage, const Buffer& buffer) {
  vk::AccessFlags flags;
  return flags;
}

void
VulkanCommandBuffer::InsertResourceBarrier(ResourceBarrier& barrier) {
  std::vector<vk::BufferMemoryBarrier> bufferMemoryBarrier;
  std::vector<vk::ImageMemoryBarrier> imageMemoryBarrier;
  for (const auto& bufferBarrier : barrier.bufferBarrier) {
    vk::BufferMemoryBarrier vulkanBufferBarrier;
    const auto& buffer = bufferBarrier.buffer;
    const auto& srcUsage = bufferBarrier.srcUsage;
    const auto& dstUsage = bufferBarrier.dstUsage;
    const auto& size = buffer.size;
    vulkanBufferBarrier.setBuffer(buffer.vulkanData->buffer);
    vulkanBufferBarrier.setOffset(0);
    vulkanBufferBarrier.setSize(size);
    vulkanBufferBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setSrcAccessMask(ConvertToVulkanAccessFlags(srcUsage, buffer));
    vulkanBufferBarrier.setDstAccessMask(ConvertToVulkanAccessFlags(dstUsage, buffer));

    bufferMemoryBarrier.push_back(vulkanBufferBarrier);
  }
}

}  // namespace Marbas
