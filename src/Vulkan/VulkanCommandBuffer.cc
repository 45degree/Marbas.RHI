#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "Buffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDescriptor.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSynchronic.hpp"
#include "common.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

void
VulkanCommandBuffer::InsertBufferBarrier(const std::vector<BufferBarrier>& barriers) {
  std::vector<vk::BufferMemoryBarrier> bufferMemoryBarrier;
  for (const auto& bufferBarrier : barriers) {
    vk::BufferMemoryBarrier vulkanBufferBarrier;
    const auto* buffer = bufferBarrier.buffer;
    auto vkBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer;
    const auto& srcUsage = bufferBarrier.waitUsage;
    const auto& dstUsage = bufferBarrier.dstUsage;
    const auto& size = buffer->size;
    vulkanBufferBarrier.setBuffer(vkBuffer);
    vulkanBufferBarrier.setOffset(0);
    vulkanBufferBarrier.setSize(size);
    vulkanBufferBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanBufferBarrier.setSrcAccessMask(ConvertToVulkanBufferAccess(srcUsage, *buffer));
    vulkanBufferBarrier.setDstAccessMask(ConvertToVulkanBufferAccess(dstUsage, *buffer));

    bufferMemoryBarrier.push_back(vulkanBufferBarrier);
  }

  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
                                  vk::DependencyFlagBits::eByRegion, nullptr, bufferMemoryBarrier, nullptr);
}

void
VulkanCommandBuffer::InsertImageBarrier(const std::vector<ImageBarrier>& barriers) {
  std::vector<vk::ImageMemoryBarrier> imageMemoryBarrier;

  for (const auto& imageBarrier : barriers) {
    vk::ImageMemoryBarrier vulkanImageBarrier;
    const auto* vulkanImage = static_cast<const VulkanImage*>(imageBarrier.image);
    const auto& srcUsage = imageBarrier.waitUsage;
    const auto& dstUsage = imageBarrier.dstUsage;
    vulkanImageBarrier.setImage(vulkanImage->image);
    vulkanImageBarrier.setDstQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcQueueFamilyIndex(m_queueFamily);
    vulkanImageBarrier.setSrcAccessMask(ConvertToVulkanImageAccess(srcUsage));
    vulkanImageBarrier.setDstAccessMask(ConvertToVulkanImageAccess(dstUsage));
    vulkanImageBarrier.setOldLayout(vk::ImageLayout::eGeneral);
    vulkanImageBarrier.setNewLayout(vk::ImageLayout::eGeneral);

    vk::ImageSubresourceRange range;
    range.setAspectMask(vulkanImage->aspect);
    range.setBaseArrayLayer(0);
    range.setBaseMipLevel(0);
    range.setLayerCount(vulkanImage->arrayLayer);
    range.setLevelCount(vulkanImage->mipMapLevel);
    vulkanImageBarrier.setSubresourceRange(range);

    imageMemoryBarrier.push_back(vulkanImageBarrier);
  }
  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eNone,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

void
VulkanCommandBuffer::TransformImageState(Image* image, ImageState srcState, ImageState dstState) {
  vk::ImageMemoryBarrier imageMemoryBarrier;
  const auto* vulkanImage = static_cast<VulkanImage*>(image);

  imageMemoryBarrier.setImage(vulkanImage->image);
  imageMemoryBarrier.setDstQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcQueueFamilyIndex(m_queueFamily);
  imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite);
  imageMemoryBarrier.setOldLayout(vulkanImage->currentLayout);
  imageMemoryBarrier.setNewLayout(ConvertToVulkanImageLayout(dstState));

  vk::ImageSubresourceRange range;
  range.setAspectMask(vulkanImage->aspect);
  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLayerCount(vulkanImage->arrayLayer);
  range.setLevelCount(vulkanImage->mipMapLevel);
  imageMemoryBarrier.setSubresourceRange(range);

  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, imageMemoryBarrier);
}

void
VulkanCommandBuffer::BindDescriptorSet(const Pipeline* pipeline, int first, std::span<DescriptorSet*> descriptors) {
  const auto* vulkanPipeline = static_cast<const VulkanPipeline*>(pipeline);
  vk::PipelineBindPoint vkPipelineBindPoint;
  if (vulkanPipeline->pipelineType == PipelineType::GRAPHICS) {
    vkPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  } else if (vulkanPipeline->pipelineType == PipelineType::COMPUTE) {
    vkPipelineBindPoint = vk::PipelineBindPoint::eCompute;
  }

  std::vector<vk::DescriptorSet> vkDescriptorSets;
  std::transform(descriptors.begin(), descriptors.end(), std::back_inserter(vkDescriptorSets),
                 [](auto* descriptor) { return static_cast<VulkanDescriptorSet*>(descriptor)->vkDescriptorSet; });

  m_commandBuffer.bindDescriptorSets(vkPipelineBindPoint, vulkanPipeline->vkPipelineLayout, first, vkDescriptorSets,
                                     nullptr);
}

void
VulkanCommandBuffer::BindVertexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindVertexBuffers(0, vkBuffer, static_cast<vk::DeviceSize>(0));
}

void
VulkanCommandBuffer::BindIndexBuffer(Buffer* buffer) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  auto& vkBuffer = vulkanBuffer->vkBuffer;
  m_commandBuffer.bindIndexBuffer(vkBuffer, 0, vk::IndexType::eUint32);
}

void
VulkanCommandBuffer::Begin() {
  vk::CommandBufferBeginInfo vkBeginInfo;
  vkBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  m_commandBuffer.begin(vkBeginInfo);
}

void
VulkanCommandBuffer::End() {
  m_commandBuffer.end();
}

void
VulkanCommandBuffer::BeginPipeline(Pipeline* pipeline, FrameBuffer* frameBuffer,
                                   const std::array<float, 4>& clearColor) {
  auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
  auto* vulkanFrameBuffer = static_cast<VulkanFrameBuffer*>(frameBuffer);
  auto vkRenderPass = vulkanPipeline->vkRenderPass;
  auto vkPipeline = vulkanPipeline->vkPipeline;
  auto vkFramebuffer = vulkanFrameBuffer->vkFrameBuffer;
  const auto& height = frameBuffer->height;
  const auto& width = frameBuffer->width;

  vk::RenderPassBeginInfo vkRenderPassBeginInfo;
  vk::ClearColorValue clearColorValue(clearColor);
  vk::ClearValue clearValue(clearColorValue);

  vkRenderPassBeginInfo.setRenderPass(vkRenderPass);
  vkRenderPassBeginInfo.setFramebuffer(vkFramebuffer);
  vkRenderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)));
  vkRenderPassBeginInfo.setClearValues(clearValue);

  m_commandBuffer.beginRenderPass(vkRenderPassBeginInfo, vk::SubpassContents::eInline);

  if (vulkanPipeline->pipelineType == PipelineType::GRAPHICS) {
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vkPipeline);
  } else if (vulkanPipeline->pipelineType == PipelineType::COMPUTE) {
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, vkPipeline);
  }
}

void
VulkanCommandBuffer::EndPipeline(Pipeline* pipeline) {
  m_commandBuffer.endRenderPass();
}

void
VulkanCommandBuffer::SetViewports(std::span<ViewportInfo> viewportInfos) {
  std::vector<vk::Viewport> vkViewports;
  std::transform(viewportInfos.begin(), viewportInfos.end(), std::back_inserter(vkViewports), [](auto& viewportInfo) {
    vk::Viewport vkViewport;
    vkViewport.setHeight(viewportInfo.height);
    vkViewport.setWidth(viewportInfo.width);
    vkViewport.setX(viewportInfo.x);
    vkViewport.setY(viewportInfo.y);
    vkViewport.setMinDepth(viewportInfo.minDepth);
    vkViewport.setMaxDepth(viewportInfo.maxDepth);
    return vkViewport;
  });

  m_commandBuffer.setViewport(0, vkViewports);
}

void
VulkanCommandBuffer::SetScissors(std::span<ScissorInfo> scissorInfos) {
  std::vector<vk::Rect2D> vkScissors;
  std::transform(scissorInfos.begin(), scissorInfos.end(), std::back_inserter(vkScissors), [](auto& scissorInfo) {
    vk::Rect2D scissor;
    scissor.setOffset(vk::Offset2D(scissorInfo.x, scissorInfo.y));
    scissor.setExtent(vk::Extent2D(scissorInfo.width, scissorInfo.height));
    return scissor;
  });
  m_commandBuffer.setScissor(0, vkScissors);
}

void
VulkanCommandBuffer::Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) {
  std::vector<vk::Semaphore> vkWaitSemaphore, vkSignalSemaphore;
  std::transform(waitSemaphore.begin(), waitSemaphore.end(), std::back_inserter(vkWaitSemaphore),
                 [](auto* semaphore) { return static_cast<VulkanSemaphore*>(semaphore)->semaphore; });
  std::transform(signalSemaphore.begin(), signalSemaphore.end(), std::back_inserter(vkSignalSemaphore),
                 [](auto* semaphore) { return static_cast<VulkanSemaphore*>(semaphore)->semaphore; });

  vk::SubmitInfo vkSubmitInfo;
  vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eAllCommands;
  vkSubmitInfo.setSignalSemaphores(vkSignalSemaphore);
  vkSubmitInfo.setWaitSemaphores(vkWaitSemaphore);
  vkSubmitInfo.setCommandBuffers(m_commandBuffer);
  vkSubmitInfo.setWaitDstStageMask(waitDstStage);

  auto* vulkanFence = static_cast<VulkanFence*>(fence);
  if (vulkanFence != nullptr) {
    m_queue.submit(vkSubmitInfo, vulkanFence->m_fence);
  } else {
    m_queue.submit(vkSubmitInfo);
  }
}

}  // namespace Marbas
