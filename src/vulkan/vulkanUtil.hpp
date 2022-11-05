#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "Image.hpp"
#include "common.hpp"

namespace Marbas {

// TODO:
FORCE_INLINE vk::Format
ConvertToVulkanFormat(ImageFormat format) {
  switch (format) {
    case ImageFormat::RED:
    case ImageFormat::RG:
    case ImageFormat::RGB:
    case ImageFormat::BGR:
    case ImageFormat::RGBA:
    case ImageFormat::BGRA:
    case ImageFormat::R32:
    case ImageFormat::RG16F:
    case ImageFormat::RG32F:
    case ImageFormat::RGB16F:
    case ImageFormat::RGB32F:
    case ImageFormat::DEPTH:
      break;
  }
  return vk::Format::eR32G32B32A32Uint;
}

FORCE_INLINE vk::ImageLayout
ConvertToVulkanImageLayout(const ImageState& state) {
  switch (state) {
    case ImageState::SHADER_READ:
      return vk::ImageLayout::eShaderReadOnlyOptimal;
    case ImageState::RENDER_TARGET:
      return vk::ImageLayout::eColorAttachmentOptimal;
    case ImageState::PRESENT:
      return vk::ImageLayout::ePresentSrcKHR;
    case ImageState::DEPTH:
      return vk::ImageLayout::eDepthAttachmentOptimal;
  }
  return vk::ImageLayout::eColorAttachmentOptimal;
}

FORCE_INLINE vk::AccessFlags
ConvertToVulkanImageAccess(uint32_t usage) {
  vk::AccessFlags flags;
  if (usage | ImageUsageFlags::TRANSFER_SRC) {
    flags |= vk::AccessFlagBits::eTransferRead;
  }
  if (usage | ImageUsageFlags::TRANSFER_DST) {
    flags |= vk::AccessFlagBits::eTransferWrite;
  }
  if (usage | ImageUsageFlags::SHADER_READ) {
    flags |= vk::AccessFlagBits::eShaderRead;
  }
  if (usage | ImageUsageFlags::DEPTH) {
    flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
  }
  if (usage | ImageUsageFlags::RENDER_TARGET) {
    flags |= vk::AccessFlagBits::eColorAttachmentWrite;
  }

  return flags;
}

FORCE_INLINE vk::AccessFlags
ConvertToVulkanBufferAccess(uint32_t usage, const Buffer& buffer) {
  vk::AccessFlags flags;
  if (usage | BufferUsageFlags::TRANSFER_SRC) {
    flags |= vk::AccessFlagBits::eTransferRead;
  }
  if (usage | BufferUsageFlags::TRANSFER_DST) {
    flags |= vk::AccessFlagBits::eTransferWrite;
  }

  if (usage | BufferUsageFlags::READ) {
    switch (buffer.bufferType) {
      case BufferType::VERTEX_BUFFER:
        flags |= vk::AccessFlagBits::eVertexAttributeRead;
        break;
      case BufferType::INDEX_BUFFER:
        flags |= vk::AccessFlagBits::eIndexRead;
        break;
      case BufferType::UNIFORM_BUFFER:
        flags |= vk::AccessFlagBits::eUniformRead;
        break;
      case BufferType::STORAGE_BUFFER:
        flags |= vk::AccessFlagBits::eShaderRead;
        break;
    }
  }

  if (usage | BufferUsageFlags::WRITE) {
    switch (buffer.bufferType) {
      case BufferType::VERTEX_BUFFER:
      case BufferType::INDEX_BUFFER:
      case BufferType::UNIFORM_BUFFER:
        flags |= vk::AccessFlagBits::eMemoryWrite;
        break;
      case BufferType::STORAGE_BUFFER:
        flags |= vk::AccessFlagBits::eShaderWrite;
        break;
    }
  }

  return flags;
}

FORCE_INLINE void
ConvetToVulkanSampleCount() {}

}  // namespace Marbas
