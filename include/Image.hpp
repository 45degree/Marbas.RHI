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

namespace Marbas {

struct ImageUsageFlags {
  enum value : uint32_t {
    SHADER_READ = 1,
    RENDER_TARGET = 1 << 1,
    TRANSFER_SRC = 1 << 2,
    TRANSFER_DST = 1 << 3,
    PRESENT = 1 << 4,
    DEPTH = 1 << 5,
  };
};

enum class ImageState {
  SHADER_READ,
  RENDER_TARGET,
  PRESENT,
  DEPTH,
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
  RGB,
  BGR,
  RGBA,
  BGRA,
  R32F,
  RG16F,
  RG32F,
  RGB16F,
  RGB32F,
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
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channel = 1;
  ImageFormat format = ImageFormat::RGBA;
  uint32_t mipMapLevel = 1;
  uint32_t usage = ImageUsageFlags::SHADER_READ | ImageUsageFlags::RENDER_TARGET;
  std::variant<CubeMapImageDesc, CubeMapArrayImageDesc, Image2DDesc, Image2DArrayDesc> imageDesc;
};

struct Image {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  ImageFormat format = ImageFormat::RGBA;
  uint32_t mipMapLevel = 1;
  uint32_t usage = ImageUsageFlags::SHADER_READ | ImageUsageFlags::RENDER_TARGET;
  uint32_t arrayLayer = 1;
};

struct ImageView {};

}  // namespace Marbas
