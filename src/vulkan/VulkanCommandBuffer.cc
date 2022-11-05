#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "Buffer.hpp"
#include "VulkanImage.hpp"
#include "common.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

void
VulkanCommandBuffer::InsertBufferBarrier(const std::vector<BufferBarrier>& barriers) {
  std::vector<vk::BufferMemoryBarrier> bufferMemoryBarrier;
  for (const auto& bufferBarrier : barriers) {
    vk::BufferMemoryBarrier vulkanBufferBarrier;
    const auto& buffer = bufferBarrier.buffer;
    const auto& srcUsage = bufferBarrier.waitUsage;
    const auto& dstUsage = bufferBarrier.dstUsage;
    const auto& size = buffer.size;
    vulkanBufferBarrier.setBuffer(buffer.vulkanData->buffer);
    vulkanBufferBarrier.setOffset(0);
    vulkanBufferBarrier.setSize(size);
    vulkanBufferBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setSrcAccessMask(ConvertToVulkanBufferAccess(srcUsage, buffer));
    vulkanBufferBarrier.setDstAccessMask(ConvertToVulkanBufferAccess(dstUsage, buffer));

    bufferMemoryBarrier.push_back(vulkanBufferBarrier);
  }

  m_commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
      vk::DependencyFlagBits::eByRegion, nullptr, bufferMemoryBarrier, nullptr);
}

void
VulkanCommandBuffer::InsertImageBarrier(const std::vector<ImageBarrier>& barriers) {
  std::vector<vk::ImageMemoryBarrier> imageMemoryBarrier;

  for (const auto& imageBarrier : barriers) {
    vk::ImageMemoryBarrier vulkanImageBarrier;
    const auto& image = imageBarrier.image;
    const auto& srcUsage = imageBarrier.waitUsage;
    const auto& dstUsage = imageBarrier.dstUsage;
    vulkanImageBarrier.setImage(image.vulkanData->image);
    vulkanImageBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcAccessMask(ConvertToVulkanImageAccess(srcUsage));
    vulkanImageBarrier.setDstAccessMask(ConvertToVulkanImageAccess(dstUsage));
    vulkanImageBarrier.setOldLayout(vk::ImageLayout::eGeneral);
    vulkanImageBarrier.setNewLayout(vk::ImageLayout::eGeneral);

    vk::ImageSubresourceRange range;
    range.setAspectMask(image.vulkanData->aspect);
    range.setBaseArrayLayer(0);
    range.setBaseMipLevel(0);
    range.setLayerCount(image.vulkanData->arrayLayer);
    range.setLevelCount(image.mipMapLevel);
    vulkanImageBarrier.setSubresourceRange(range);

    imageMemoryBarrier.push_back(vulkanImageBarrier);
  }
  m_commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
      vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

void
VulkanCommandBuffer::TransformImageState(Image& image, ImageState srcState, ImageState dstState) {
  vk::ImageMemoryBarrier imageMemoryBarrier;

  imageMemoryBarrier.setImage(image.vulkanData->image);
  imageMemoryBarrier.setDstQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setOldLayout(image.vulkanData->currentLayout);
  imageMemoryBarrier.setNewLayout(ConvertToVulkanImageLayout(dstState));

  vk::ImageSubresourceRange range;
  range.setAspectMask(image.vulkanData->aspect);
  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLayerCount(image.vulkanData->arrayLayer);
  range.setLevelCount(image.mipMapLevel);
  imageMemoryBarrier.setSubresourceRange(range);

  m_commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
      vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

}  // namespace Marbas
