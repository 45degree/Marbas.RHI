#pragma once

#include <glog/logging.h>

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "DescriptorSet.hpp"
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

FORCE_INLINE vk::Format
ConvertToVulkanFormat(ElementType type) {
  switch (type) {
    case ElementType::R32_SFLOAT:
      return vk::Format::eR32Sfloat;
    case ElementType::R32G32_SFLOAT:
      return vk::Format::eR32G32Sfloat;
    case ElementType::R32G32B32_SFLOAT:
      return vk::Format::eR32G32B32Sfloat;
    case ElementType::R32G32B32A32_SFLOAT:
      return vk::Format::eR32G32B32A32Sfloat;
    case ElementType::R32G32_SINT:
      return vk::Format::eR32G32Sint;
    case ElementType::R32G32B32A32_UINT:
      return vk::Format::eR32G32B32A32Uint;
    case ElementType::R64_SFLOAT:
      return vk::Format::eR64Sfloat;
  }
  return vk::Format::eR32G32B32Sfloat;
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
  if (usage & ImageUsageFlags::TRANSFER_SRC) {
    flags |= vk::AccessFlagBits::eTransferRead;
  }
  if (usage & ImageUsageFlags::TRANSFER_DST) {
    flags |= vk::AccessFlagBits::eTransferWrite;
  }
  if (usage & ImageUsageFlags::SHADER_READ) {
    flags |= vk::AccessFlagBits::eShaderRead;
  }
  if (usage & ImageUsageFlags::DEPTH) {
    flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
  }
  if (usage & ImageUsageFlags::RENDER_TARGET) {
    flags |= vk::AccessFlagBits::eColorAttachmentWrite;
  }

  return flags;
}

FORCE_INLINE vk::AccessFlags
ConvertToVulkanBufferAccess(uint32_t usage, const Buffer& buffer) {
  vk::AccessFlags flags;
  if (usage & BufferUsageFlags::TRANSFER_SRC) {
    flags |= vk::AccessFlagBits::eTransferRead;
  }
  if (usage & BufferUsageFlags::TRANSFER_DST) {
    flags |= vk::AccessFlagBits::eTransferWrite;
  }

  if (usage & BufferUsageFlags::READ) {
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

  if (usage & BufferUsageFlags::WRITE) {
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

FORCE_INLINE vk::ImageUsageFlags
ConvertToVulkanImageUsage(uint32_t usage) {
  vk::ImageUsageFlags flags;
  if (usage & ImageUsageFlags::RENDER_TARGET) {
    flags |= vk::ImageUsageFlagBits::eColorAttachment;
  }
  if (usage & ImageUsageFlags::DEPTH) {
    flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
  }
  if (usage & ImageUsageFlags::TRANSFER_SRC) {
    flags |= vk::ImageUsageFlagBits::eTransferSrc;
  }
  return flags;
}

FORCE_INLINE vk::DescriptorType
ConvertToVulkanDescriptorType(DescriptorType type) {
  switch (type) {
    case DescriptorType::UNIFORM_BUFFER:
      return vk::DescriptorType::eUniformBuffer;
    case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
      return vk::DescriptorType::eUniformBufferDynamic;
    case DescriptorType::IMAGE:
      return vk::DescriptorType::eCombinedImageSampler;
  }
  DLOG_ASSERT(false);
}

FORCE_INLINE vk::DescriptorSetLayoutBinding
ConvertToVulkanDescriptorLayoutBinding(const DescriptorSetLayoutBinding& binding) {
  vk::DescriptorSetLayoutBinding vkBinding;
  vkBinding.setDescriptorType(ConvertToVulkanDescriptorType(binding.descriptorType));
  vkBinding.setDescriptorCount(binding.count);
  vkBinding.setBinding(binding.count);

  // TODO:
  vkBinding.setStageFlags(vk::ShaderStageFlagBits::eAll);
  return vkBinding;
}

FORCE_INLINE void
ConvetToVulkanSampleCount() {}

}  // namespace Marbas
