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

#include <cstddef>

#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanImage.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

static uint32_t
GetChannelFromFormat(const ImageFormat& format) {
  switch (format) {
    case ImageFormat::RED:
    case ImageFormat::R32F:
    case ImageFormat::DEPTH:
      return 1;
    case ImageFormat::RG:
    case ImageFormat::RG16F:
    case ImageFormat::RG32F:
      return 2;
    case ImageFormat::RGB:
    case ImageFormat::BGR:
    case ImageFormat::RGB16F:
    case ImageFormat::RGB32F:
    case ImageFormat::RGB_SRGB:
      return 3;
    case ImageFormat::RGBA:
    case ImageFormat::BGRA:
    case ImageFormat::RGBA_SRGB:
      return 4;
  }
}

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

void
VulkanBufferContext::UpdateBuffer(Buffer* buffer, const void* data, uint32_t size, uintptr_t offset) {
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
  if (vulkanBuffer->stageBufferMemory && vulkanBuffer->stageBuffer) {
    void* mapData = m_device.mapMemory(*vulkanBuffer->stageBufferMemory, offset, size);
    memcpy(mapData, data, size);
    m_device.unmapMemory(*vulkanBuffer->stageBufferMemory);
    CopyBuffer(*(vulkanBuffer->stageBuffer), vulkanBuffer->vkBuffer, vulkanBuffer->size);
  } else {
    void* mapData = m_device.mapMemory(vulkanBuffer->vkMemory, 0, size);
    memcpy(mapData, data, size);
    m_device.unmapMemory(vulkanBuffer->vkMemory);
  }
}

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

// TODO: improve
Image*
VulkanBufferContext::CreateImage(const ImageCreateInfo& imageCreateInfo) {
  vk::ImageCreateInfo vkCreateInfo;
  uint32_t width, height, channel;
  width = imageCreateInfo.width;
  height = imageCreateInfo.height;
  channel = GetChannelFromFormat(imageCreateInfo.format);

  auto imageSize = width * height * channel;

  auto* vulkanImage = new VulkanImage();
  vulkanImage->width = width;
  vulkanImage->height = height;
  vulkanImage->usage = imageCreateInfo.usage;
  vulkanImage->mipMapLevel = imageCreateInfo.mipMapLevel;
  vulkanImage->format = imageCreateInfo.format;

  if (imageCreateInfo.usage & ImageUsageFlags::DEPTH) {
    // when image be used as a depth image, it can't be used as a color render target
    DLOG_ASSERT(!(imageCreateInfo.usage & ImageUsageFlags::COLOR_RENDER_TARGET));
    DLOG_ASSERT(!(imageCreateInfo.usage & ImageUsageFlags::PRESENT));
    vulkanImage->vkAspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
  } else {
    vulkanImage->vkAspect = vk::ImageAspectFlagBits::eColor;
  }

  vkCreateInfo.setFormat(ConvertToVulkanFormat(imageCreateInfo.format));
  vkCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
  vkCreateInfo.setMipLevels(imageCreateInfo.mipMapLevel);

  // TODO:
  vkCreateInfo.setTiling(vk::ImageTiling::eOptimal);
  vkCreateInfo.setSamples(ConvertToVulkanSampleCount(imageCreateInfo.sampleCount));

  // clang-format off
  std::visit([&](auto&& imageDesc) {
    using T = std::decay_t<decltype(imageDesc)>;
    if constexpr (std::is_same_v<T, Image2DDesc>) {
      vulkanImage->depth = 1;
      vkCreateInfo.setImageType(vk::ImageType::e2D);
      vkCreateInfo.setExtent(vk::Extent3D(width, height, 1));
      vkCreateInfo.setArrayLayers(1);
      vulkanImage->arrayLayer = 1;
    } else if constexpr (std::is_same_v<T, Image2DArrayDesc>) {
      vkCreateInfo.setImageType(vk::ImageType::e2D);
      vkCreateInfo.setArrayLayers(imageDesc.arraySize);
      vulkanImage->arrayLayer = imageDesc.arraySize;
    } else if constexpr (std::is_same_v<T, CubeMapImageDesc>) {
      vkCreateInfo.setImageType(vk::ImageType::e2D);
      vkCreateInfo.setArrayLayers(6);
      vkCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
      vulkanImage->arrayLayer = 6;
    } else if constexpr (std::is_same_v<T, CubeMapArrayImageDesc>) {
      vkCreateInfo.setImageType(vk::ImageType::e2D);
      vkCreateInfo.setArrayLayers(6 * imageDesc.arraySize);
      vkCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
      vulkanImage->arrayLayer = 6 * imageDesc.arraySize;
    }
  },imageCreateInfo.imageDesc);
  // clang-format on

  vkCreateInfo.setUsage(ConvertToVulkanImageUsage(imageCreateInfo.usage));
  vkCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

  vulkanImage->vkImage = m_device.createImage(vkCreateInfo);

  /**
   * alloca memory
   */
  auto requirements = m_device.getImageMemoryRequirements(vulkanImage->vkImage);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(requirements.size);
  allocInfo.setMemoryTypeIndex(FindMemoryType(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
  vulkanImage->vkImageMemory = m_device.allocateMemory(allocInfo, nullptr);
  m_device.bindImageMemory(vulkanImage->vkImage, vulkanImage->vkImageMemory, 0);

  /**
   * create staging buffer
   */
  vk::MemoryPropertyFlags memPropertyFlag;
  memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostVisible;
  memPropertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;
  auto [StagingBuffer, StagingMemory] = CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, memPropertyFlag);

  vulkanImage->vkStagingBuffer = StagingBuffer;
  vulkanImage->vkStagingBufferMemory = StagingMemory;

  return vulkanImage;
}

// TODO: improve
void
VulkanBufferContext::UpdateImage(const UpdateImageInfo& updateInfo) {
  auto* vulkanImage = static_cast<VulkanImage*>(updateInfo.image);
  void* mapData = m_device.mapMemory(vulkanImage->vkStagingBufferMemory, 0, updateInfo.dataSize);
  memcpy(mapData, updateInfo.data, updateInfo.dataSize);
  m_device.unmapMemory(vulkanImage->vkStagingBufferMemory);

  ConvertImageState(updateInfo.image, ImageState::UNDEFINED, ImageState::TRANSFER_DST);

  vk::BufferImageCopy range;
  range.setBufferImageHeight(0);
  range.setBufferRowLength(0);
  range.setBufferOffset(0);

  range.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
  range.imageSubresource.setBaseArrayLayer(0);
  range.imageSubresource.setMipLevel(updateInfo.level);
  range.imageSubresource.setLayerCount(vulkanImage->arrayLayer);

  range.imageOffset = vk::Offset3D(updateInfo.xOffset, updateInfo.yOffset, updateInfo.zOffset);
  range.imageExtent = vk::Extent3D(updateInfo.width, updateInfo.height, updateInfo.depth);

  CopyBufferToImage(vulkanImage->vkStagingBuffer, vulkanImage->vkImage, range);
  ConvertImageState(updateInfo.image, ImageState::TRANSFER_DST, ImageState::SHADER_READ);
}

void
VulkanBufferContext::GenerateMipmap(Image* image, uint32_t mipmapLevel) {
  ConvertImageState(image, Marbas::ImageState::UNDEFINED, Marbas::ImageState::TRANSFER_DST);
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  const auto& vkImage = vulkanImage->vkImage;

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

  vk::ImageMemoryBarrier barrier;
  barrier.setImage(vkImage);
  barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;  // TODO: depth?
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = vulkanImage->arrayLayer;
  barrier.subresourceRange.setLevelCount(1);

  // TODO:
  int32_t mipWidth = vulkanImage->width;
  int32_t mipHeight = vulkanImage->height;

  for (uint32_t i = 1; i < mipmapLevel; i++) {
    barrier.subresourceRange.setBaseMipLevel(i - 1);
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

    vk::ImageBlit blit;
    blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
    blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = vulkanImage->arrayLayer;

    blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
    blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = vulkanImage->arrayLayer;

    commandBuffer.blitImage(vkImage, vk::ImageLayout::eTransferSrcOptimal, vkImage,
                            vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

    barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
    barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                  vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);
    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
  }
  barrier.subresourceRange.baseMipLevel = mipmapLevel - 1;
  barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
  barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
  barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(commandBuffer);
  m_transferQueue.submit(submitInfo);
  m_transferQueue.waitIdle();

  m_device.freeCommandBuffers(m_temporaryCommandPool, commandBuffer);
}

// TODO: improve
void
VulkanBufferContext::ConvertImageState(Image* image, ImageState srcState, ImageState dstState) {
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  auto vkImage = vulkanImage->vkImage;

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

  vk::ImageMemoryBarrier barrier{};
  vk::ImageSubresourceRange range;
  range.setAspectMask(vulkanImage->vkAspect);
  range.setBaseArrayLayer(0);
  range.setLayerCount(vulkanImage->arrayLayer);
  range.setBaseMipLevel(0);
  range.setLevelCount(image->mipMapLevel);

  barrier.setOldLayout(ConvertToVulkanImageLayout(srcState));
  barrier.setNewLayout(ConvertToVulkanImageLayout(dstState));
  barrier.setImage(vkImage);
  barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  barrier.setSubresourceRange(range);

  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                                vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

  commandBuffer.end();
  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(commandBuffer);
  m_transferQueue.submit(submitInfo);
  m_transferQueue.waitIdle();
  m_device.freeCommandBuffers(m_temporaryCommandPool, commandBuffer);
}

void
VulkanBufferContext::DestroyImage(Image* image) {
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  m_device.destroyImage(vulkanImage->vkImage);
  m_device.destroyBuffer(vulkanImage->vkStagingBuffer);
  m_device.freeMemory(vulkanImage->vkImageMemory);
  m_device.freeMemory(vulkanImage->vkStagingBufferMemory);

  delete image;
}

ImageView*
VulkanBufferContext::CreateImageView(const ImageViewCreateInfo& createInfo) {
  auto* imageView = new VulkanImageView();
  auto* vulkanImage = static_cast<VulkanImage*>(createInfo.image);
  const auto& vkImage = vulkanImage->vkImage;

  vk::ImageViewCreateInfo vkImageViewCreateInfo;
  vkImageViewCreateInfo.setImage(vkImage);

  vkImageViewCreateInfo.setFormat(ConvertToVulkanFormat(vulkanImage->format));

  switch (createInfo.type) {
    case ImageViewType::TEXTURE2D:
      vkImageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
      break;
    case ImageViewType::CUBEMAP:
      vkImageViewCreateInfo.setViewType(vk::ImageViewType::eCube);
      break;
    case ImageViewType::TEXTURE2D_ARRAY:
      vkImageViewCreateInfo.setViewType(vk::ImageViewType::e2DArray);
      break;
    case ImageViewType::CUBEMAP_ARRAY:
      vkImageViewCreateInfo.setViewType(vk::ImageViewType::eCubeArray);
      break;
  }

  vk::ImageSubresourceRange range;
  range.setAspectMask(vulkanImage->vkAspect);
  range.setBaseArrayLayer(createInfo.baseArrayLayer);
  range.setBaseMipLevel(createInfo.baseLevel);
  range.setLayerCount(createInfo.layerCount);
  range.setLevelCount(createInfo.levelCount);
  vkImageViewCreateInfo.setSubresourceRange(range);

  auto vkImageView = m_device.createImageView(vkImageViewCreateInfo);
  imageView->vkImageView = vkImageView;

  return imageView;
}

void
VulkanBufferContext::DestroyImageView(ImageView* imageView) {
  auto* vulkanImageView = static_cast<VulkanImageView*>(imageView);
  m_device.destroyImageView(vulkanImageView->vkImageView);

  delete vulkanImageView;
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

void
VulkanBufferContext::CopyBufferToImage(vk::Buffer srcBuffer, vk::Image image, const vk::BufferImageCopy& copyRange) {
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

  commandBuffer.copyBufferToImage(srcBuffer, image, vk::ImageLayout::eTransferDstOptimal, copyRange);

  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(commandBuffer);
  m_transferQueue.submit(submitInfo);
  m_transferQueue.waitIdle();

  m_device.freeCommandBuffers(m_temporaryCommandPool, commandBuffer);
}

vk::Format
VulkanBufferContext::FindSupportFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                       vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {
    auto props = m_physicalDevice.getFormatProperties(format);

    if ((props.linearTilingFeatures & features) == features) {
      if (tiling == vk::ImageTiling::eLinear) {
        return format;
      } else if (tiling == vk::ImageTiling::eOptimal) {
        return format;
      }
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

}  // namespace Marbas
