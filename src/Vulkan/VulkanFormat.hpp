#pragma once

#include <vulkan/vulkan.hpp>

#include "Image.hpp"

namespace Marbas {

struct FormatDesc {
  vk::Format vkFormat;
  uint32_t bytesPerBlock;
  uint32_t channelCount;
  struct {
    bool isDepth;
    bool isStencil;
    bool isCompressed;
  };
  struct {
    uint32_t width;
    uint32_t height;
  } compressionRatio;
  int numChannelBits[4];
};

uint32_t
GetMipLevelPackedDataSize(ImageFormat format, uint32_t w, uint32_t h, uint32_t d);

vk::Format
GetVkFormat(ImageFormat format);

}  // namespace Marbas
