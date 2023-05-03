/**
 * Copyright 2022.11.1 45degree
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

#include <cstdint>
#include <variant>

#include "Sampler.hpp"

namespace Marbas {

struct ImageUsageFlags {
  enum value : uint32_t {
    SHADER_READ = 1 << 0,
    COLOR_RENDER_TARGET = 1 << 1,
    DEPTH_STENCIL = 1 << 2,
    STORAGE = 1 << 3,
  };
};

enum class ImageViewType {
  TEXTURE2D,
  CUBEMAP,
  TEXTURE2D_ARRAY,
  CUBEMAP_ARRAY,
};

enum class ImageFormat {
  RED,
  RG,
  RGBA,
  BGRA,
  R32F,
  RG16F,
  RG32F,
  RGBA16F,
  RGBA32F,
  DEPTH,
  RGBA_SRGB,
};

struct CubeMapImageDesc {};
struct CubeMapArrayImageDesc {
  uint32_t arraySize = 1;
};
struct Image2DDesc {};
struct Image2DArrayDesc {
  uint32_t arraySize = 1;
};

struct ImageCreateInfo {
  uint32_t width = 256;
  uint32_t height = 256;
  ImageFormat format = ImageFormat::RGBA;
  SampleCount sampleCount = SampleCount::BIT1;
  uint32_t mipMapLevel = 1;
  uint32_t usage = ImageUsageFlags::SHADER_READ | ImageUsageFlags::COLOR_RENDER_TARGET;
  std::variant<CubeMapImageDesc, CubeMapArrayImageDesc, Image2DDesc, Image2DArrayDesc> imageDesc = Image2DDesc{};
};

struct Image {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  ImageFormat format = ImageFormat::RGBA;
  uint32_t mipMapLevel = 1;
  uint32_t usage = ImageUsageFlags::SHADER_READ | ImageUsageFlags::COLOR_RENDER_TARGET;
  uint32_t arrayLayer = 1;
  SampleCount sampleCount = SampleCount::BIT1;

  uint32_t
  GetHeight(uint32_t mipmapLevel) {
    return (mipmapLevel < this->mipMapLevel) ? std::max(1U, height >> mipmapLevel) : 0;
  }

  uint32_t
  GetWidth(uint32_t mipmapLevel) {
    return (mipmapLevel < this->mipMapLevel) ? std::max(1U, width >> mipmapLevel) : 0;
  }

  uint32_t
  GetDepth(uint32_t mipmapLevel) {
    return (mipmapLevel < this->mipMapLevel) ? std::max(1U, depth >> mipmapLevel) : 0;
  }
};

struct ImageSubresourceDesc {
  Image* image;
  uint32_t baseArrayLayer;
  uint32_t layerCount;
  uint32_t mipmapLevel;
};

struct ImageViewCreateInfo {
  Image* image;
  ImageViewType type;
  uint32_t baseLevel;
  uint32_t levelCount;
  uint32_t baseArrayLayer;
  uint32_t layerCount;
};

struct ImageView {};

}  // namespace Marbas
