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

#include <span>

#include "Buffer.hpp"
#include "CommandBuffer.hpp"

namespace Marbas {

struct UpdateImageInfo {
  Image* image;
  ImageState srcImageState = ImageState::UNDEFINED;
  ImageState dstImageState;
  uint32_t level;
  int32_t xOffset;
  int32_t yOffset;
  int32_t zOffset;
  int32_t width;
  int32_t height;
  int32_t depth;
  void* data;
  size_t size;
};

class BufferContext {
 public:
  BufferContext() = default;
  virtual ~BufferContext() = default;

 public:
  virtual Buffer*
  CreateBuffer(BufferType bufferType, const void* data, uint32_t size, bool isStatic) = 0;

  virtual void
  UpdateBuffer(Buffer* buffer, const void* data, uint32_t size, uintptr_t offset) = 0;

  virtual void
  DestroyBuffer(Buffer* buffer) = 0;

 public:
  virtual Image*
  CreateImage(const ImageCreateInfo& createInfo) = 0;

  virtual void
  UpdateImage(const UpdateImageInfo& updateInfo) = 0;

  virtual void
  ConvertImageState(Image* image, ImageState srcState, ImageState dstState) = 0;

  virtual void
  GenerateMipmap(Image* image, uint32_t mipmapLevel) = 0;

  virtual void
  DestroyImage(Image* image) = 0;

  virtual ImageView*
  CreateImageView(const ImageViewCreateInfo& createInfo) = 0;

  virtual void
  DestroyImageView(ImageView* imageView) = 0;

 public:
  virtual CommandPool*
  CreateCommandPool(CommandBufferUsage usage) = 0;

  virtual void
  DestroyCommandPool(CommandPool* commandPool) = 0;

  virtual CommandBuffer*
  CreateCommandBuffer(CommandPool* commandPool) = 0;

  virtual void
  DestroyCommandBuffer(CommandPool* commandPool, CommandBuffer* commandBuffer) = 0;
};

}  // namespace Marbas
