#include "VulkanFormat.hpp"

#include <cstdint>
#include <unordered_map>

#define align_to(_alignment, _val) ((((_val) + (_alignment)-1) / (_alignment)) * (_alignment))

namespace Marbas {

const static std::unordered_map<ImageFormat, FormatDesc> g_formatDesc{
    {ImageFormat::RED, {vk::Format::eR8Unorm, 1, 1, {false, false, false}, {1, 1}, {8, 0, 0, 0}}},
    {ImageFormat::RG, {vk::Format::eR8G8Unorm, 2, 2, {false, false, false}, {1, 1}, {8, 8, 0, 0}}},
    {ImageFormat::RGBA, {vk::Format::eR8G8B8A8Unorm, 4, 4, {false, false, false}, {1, 1}, {8, 8, 8, 8}}},
    {ImageFormat::BGRA, {vk::Format::eB8G8R8A8Unorm, 4, 4, {false, false, false}, {1, 1}, {8, 8, 8, 8}}},
    {ImageFormat::R32F, {vk::Format::eR32Sfloat, 4, 1, {false, false, false}, {1, 1}, {32, 0, 0, 0}}},
    {ImageFormat::RG16F, {vk::Format::eR16G16Sfloat, 4, 2, {false, false, false}, {1, 1}, {16, 16, 0, 0}}},
    {ImageFormat::RG32F, {vk::Format::eR32G32Sfloat, 8, 2, {false, false, false}, {1, 1}, {32, 32, 0, 0}}},
    {ImageFormat::RGBA16F, {vk::Format::eR16G16B16A16Sfloat, 8, 4, {false, false, false}, {1, 1}, {16, 16, 16, 16}}},
    {ImageFormat::RGBA32F, {vk::Format::eR32G32B32A32Sfloat, 16, 4, {false, false, false}, {1, 1}, {32, 32, 32, 32}}},
    {ImageFormat::DEPTH, {vk::Format::eD32SfloatS8Uint, 8, 2, {true, true, false}, {1, 1}, {32, 8, 24, 0}}},
    {ImageFormat::RGBA_SRGB, {vk::Format::eR8G8B8A8Srgb, 4, 4, {false, false, false}, {1, 1}, {8, 8, 8, 8}}},
    {ImageFormat::R32UI, {vk::Format::eR32Uint, 4, 1, {false, false, false}, {1, 1}, {32, 0, 0, 0}}},
};

uint32_t
GetMipLevelPackedDataSize(ImageFormat format, uint32_t w, uint32_t h, uint32_t d) {
  uint32_t perW = g_formatDesc.at(format).compressionRatio.width;
  uint32_t bw = align_to(perW, w) / perW;

  uint32_t perH = g_formatDesc.at(format).compressionRatio.height;
  uint32_t bh = align_to(perH, h) / perH;

  uint32_t size = bh * bw * d * g_formatDesc.at(format).bytesPerBlock;
  return size;
}

vk::Format
GetVkFormat(ImageFormat format) {
  return g_formatDesc.at(format).vkFormat;
}

}  // namespace Marbas
