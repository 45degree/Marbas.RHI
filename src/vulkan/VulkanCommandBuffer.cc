#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "Buffer.hpp"
#include "VulkanImage.hpp"
#include "common.hpp"
#include "vulkanBuffer.hpp"

namespace Marbas {

FORCE_INLINE static vk::AccessFlags
ConvertToVulkanAccessFlags(const ResourceUsage& usage, const Buffer& buffer) {
  vk::AccessFlags flags;
  return flags;
}

FORCE_INLINE static vk::AccessFlags
ConvertToVulkanAccessFlags(const ResourceUsage& usage, const Image& image) {
  vk::AccessFlags flags;
  return flags;
}

FORCE_INLINE static vk::ImageLayout
ConvertToVulkanImageLayout(const ResourceUsage& usage, const Image& image) {
  vk::ImageLayout layout;
  return layout;
}

// FORCE_INLINE static vk::ImageAspectFlags
// ConvertToVulkanImageAspectFlag(const )

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

  // image barrier
  for (const auto& imageBarrier : barrier.imageBarrier) {
    vk::ImageMemoryBarrier vulkanImageBarrier;
    const auto& image = imageBarrier.image;
    const auto& srcUsage = imageBarrier.srcUsage;
    const auto& dstUsage = imageBarrier.dstUsage;
    vulkanImageBarrier.setImage(image.vulkanData->image);
    vulkanImageBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcAccessMask(ConvertToVulkanAccessFlags(srcUsage, image));
    vulkanImageBarrier.setDstAccessMask(ConvertToVulkanAccessFlags(srcUsage, image));
    vulkanImageBarrier.setOldLayout(vk::ImageLayout::eGeneral);
    vulkanImageBarrier.setNewLayout(vk::ImageLayout::eGeneral);

    // TODO:
    vk::ImageSubresourceRange range;
    // vk::ImageSubresourceRange range;
    // vulkanImageBarrier.setSubresourceRange()
  }

  auto srcStage = vk::PipelineStageFlagBits::eNone;
  auto dstStage = srcStage;
  auto dependenceFlag = vk::DependencyFlagBits::eByRegion;
  m_commandBuffer.pipelineBarrier(srcStage, dstStage, dependenceFlag, nullptr, bufferMemoryBarrier,
                                  imageMemoryBarrier);
}

}  // namespace Marbas
