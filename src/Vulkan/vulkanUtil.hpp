/**
 * Copyright 2022.11.30 45degree
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
#pragma once

#include <glog/logging.h>

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "DescriptorSet.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "common.hpp"

namespace Marbas {

// TODO:
FORCE_INLINE vk::Format
ConvertToVulkanFormat(ImageFormat format) {
  switch (format) {
    case ImageFormat::RED:
      return vk::Format::eR8Unorm;
    case ImageFormat::RG:
      return vk::Format::eR8G8Unorm;
    case ImageFormat::RGBA:
      return vk::Format::eR8G8B8A8Unorm;
    case ImageFormat::BGRA:
      return vk::Format::eB8G8R8A8Unorm;
    case ImageFormat::R32F:
      return vk::Format::eR32Sfloat;
    case ImageFormat::RG16F:
      return vk::Format::eR16G16Sfloat;
    case ImageFormat::RG32F:
      return vk::Format::eR32G32Sfloat;
    case ImageFormat::RGBA16F:
      return vk::Format::eR16G16B16A16Sfloat;
    case ImageFormat::RGBA32F:
      return vk::Format::eR32G32B32A32Sfloat;
    case ImageFormat::DEPTH:
      return vk::Format::eD32SfloatS8Uint;
    case ImageFormat::RGBA_SRGB:
      return vk::Format::eR8G8B8A8Srgb;
  }
  return vk::Format::eR8G8B8A8Unorm;
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
GetDefaultImageLayoutFromUsage(uint32_t usage) {
  if (usage & ImageUsageFlags::SHADER_READ) {
    return vk::ImageLayout::eShaderReadOnlyOptimal;
  } else if (usage & ImageUsageFlags::COLOR_RENDER_TARGET) {
    return vk::ImageLayout::eColorAttachmentOptimal;
  } else if (usage & ImageUsageFlags::DEPTH_STENCIL) {
    return vk::ImageLayout::eDepthStencilAttachmentOptimal;
  }
  return vk::ImageLayout::eGeneral;
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
  if (usage & ImageUsageFlags::DEPTH_STENCIL) {
    flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
  }
  if (usage & ImageUsageFlags::COLOR_RENDER_TARGET) {
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
  if (usage & ImageUsageFlags::COLOR_RENDER_TARGET) {
    flags |= vk::ImageUsageFlagBits::eColorAttachment;
  }
  if (usage & ImageUsageFlags::DEPTH_STENCIL) {
    flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
  }
  if (usage & ImageUsageFlags::SHADER_READ) {
    flags |= vk::ImageUsageFlagBits::eSampled;
  }
  if (usage & ImageUsageFlags::TRANSFER_SRC) {
    flags |= vk::ImageUsageFlagBits::eTransferSrc;
  }
  if (usage & ImageUsageFlags::TRANSFER_DST) {
    flags |= vk::ImageUsageFlagBits::eTransferDst;
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

FORCE_INLINE vk::CullModeFlags
ConvertToVulkanCullMode(const CullMode& cullMode) {
  switch (cullMode) {
    case CullMode::NONE:
      return vk::CullModeFlagBits::eNone;
    case CullMode::FRONT:
      return vk::CullModeFlagBits::eFront;
    case CullMode::BACK:
      return vk::CullModeFlagBits::eBack;
    case CullMode::FRONT_AND_BACK:
      return vk::CullModeFlagBits::eFrontAndBack;
  }
  return vk::CullModeFlagBits::eFront;
}

FORCE_INLINE vk::PrimitiveTopology
ConvertToVulkanTopology(const PrimitiveTopology& topology) {
  switch (topology) {
    case PrimitiveTopology::POINT:
      return vk::PrimitiveTopology::ePointList;
    case PrimitiveTopology::LINE:
      return vk::PrimitiveTopology::eLineList;
    case PrimitiveTopology::TRIANGLE:
      return vk::PrimitiveTopology::eTriangleList;
    case PrimitiveTopology::PATCH:
      return vk::PrimitiveTopology::ePatchList;
  }
  return vk::PrimitiveTopology::eTriangleList;
}

FORCE_INLINE vk::PolygonMode
ConvertToVulkanPolyMode(const PolygonMode& polygonMode) {
  switch (polygonMode) {
    case PolygonMode::FILL:
      return vk::PolygonMode::eFill;
    case PolygonMode::POINT:
      return vk::PolygonMode::ePoint;
    case PolygonMode::LINE:
      return vk::PolygonMode::eLine;
  }
  return vk::PolygonMode::eFill;
}

FORCE_INLINE vk::FrontFace
ConvertToVulkanFrontFace(const FrontFace& frontFace) {
  switch (frontFace) {
    case FrontFace::CW:
      return vk::FrontFace::eClockwise;
    case FrontFace::CCW:
      return vk::FrontFace::eCounterClockwise;
  }
  return vk::FrontFace::eClockwise;
}

FORCE_INLINE vk::SampleCountFlagBits
ConvertToVulkanSampleCount(const SampleCount& sampleCount) {
  switch (sampleCount) {
    case SampleCount::BIT1:
      return vk::SampleCountFlagBits::e1;
    case SampleCount::BIT2:
      return vk::SampleCountFlagBits::e2;
    case SampleCount::BIT4:
      return vk::SampleCountFlagBits::e4;
    case SampleCount::BIT8:
      return vk::SampleCountFlagBits::e8;
    case SampleCount::BIT16:
      return vk::SampleCountFlagBits::e16;
    case SampleCount::BIT32:
      return vk::SampleCountFlagBits::e32;
    case SampleCount::BIT64:
      return vk::SampleCountFlagBits::e64;
  }
  return vk::SampleCountFlagBits::e1;
}

}  // namespace Marbas
