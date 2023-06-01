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

#pragma once

#include <vulkan/vulkan.hpp>

#include "BufferContext.hpp"
#include "VulkanPipelineContext.hpp"

namespace Marbas {

struct VulkanBufferContextCreateInfo final {
  vk::Device device;
  vk::PhysicalDevice physicalDevice;
  uint32_t graphicsQueueIndex = 0;
  uint32_t computeQueueIndex = 0;
  uint32_t transfermQueueIndex = 0;
  vk::Queue graphicsQueue;
  vk::Queue computeQueue;
  vk::Queue transferQueue;
  VulkanPipelineContext* pipelineCtx;
};

class VulkanBufferContext final : public BufferContext {
 public:
  explicit VulkanBufferContext(const VulkanBufferContextCreateInfo& createInfo);
  ~VulkanBufferContext() override;

 public:
  Buffer*
  CreateBuffer(BufferType bufferType, const void* data, uint32_t size, bool isStatic) override;

  void
  UpdateBuffer(Buffer* buffer, const void* data, uint32_t size, uintptr_t offset) override;

  void
  DestroyBuffer(Buffer* buffer) override;

  void
  GetBufferData(Buffer* buffer, void* data) override;

 public:
  TexelBuffer*
  CreateTexelBuffer(Buffer* buffer, ImageFormat format) override;

  void
  DestroyTexelBuffer(TexelBuffer* texelBuffer) override;

 public:
  Image*
  CreateImage(const ImageCreateInfo& createInfo) override;

  void
  UpdateImage(const UpdateImageInfo& updateInfo) override;

  void
  GetImageData(const ImageSubresourceDesc& imageRange, void* data) override;

  uint32_t
  GetImageSubresourceSize(const ImageSubresourceDesc& imageRange) override;

  void
  GenerateMipmap(Image* image, uint32_t mipmapLevel) override;

  void
  DestroyImage(Image* image) override;

  ImageView*
  CreateImageView(const ImageViewCreateInfo& createInfo) override;

  void
  DestroyImageView(ImageView* imageView) override;

 public:
  GraphicsCommandBuffer*
  CreateGraphicsCommandBuffer() override;

  ComputeCommandBuffer*
  CreateComputeCommandBuffer() override;

  void
  DestroyCommandBuffer(GraphicsCommandBuffer* commandBuffer) override;

  void
  DestroyCommandBuffer(ComputeCommandBuffer* commandBuffer) override;

 private:
  uint32_t
  FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

  std::tuple<vk::Buffer, vk::DeviceMemory>
  CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

  void
  CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

  void
  CopyBufferToImage(vk::Buffer srcBuffer, vk::Image image, const vk::BufferImageCopy& range);

  vk::Format
  FindSupportFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

  void
  ConvertImageState(Image* image, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout);

 private:
  VulkanPipelineContext* m_pipelineCtx;

  uint32_t m_graphicsQueueIndex;
  uint32_t m_computeQueueIndex;
  uint32_t m_transfermQueueIndex;
  vk::Queue m_graphicsQueue;
  vk::Queue m_computeQueue;
  vk::Queue m_transferQueue;

  vk::Device m_device;
  vk::PhysicalDevice m_physicalDevice;

  vk::CommandPool m_temporaryCommandPool;
  vk::CommandPool m_graphicsCommandPool;
  vk::CommandPool m_computeCommandPool;

  vk::Buffer m_stagingBuffer;
};

}  // namespace Marbas
