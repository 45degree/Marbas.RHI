#pragma once

#include "VulkanKtxLoader.hpp"

#include <ktx.h>

// clang-format off
#include <vulkan/vulkan.h> // it must be include before ktxvulkan
#include <ktxvulkan.h>
// clang-format on

#include "Vulkan/VulkanImage.hpp"
#include "Vulkan/VulkanRHIFactory.hpp"
#include "ktxExtension.hpp"

namespace Marbas {

Image*
VulkanKtxLoader::LoadFromPath(std::filesystem::path& path) {
  auto device = VulkanRHIFactory::GetVkDevice();
  auto gpu = VulkanRHIFactory::GetVkPhysicalDevice();

  VulkanImage image;

  ktxTexture* kTexture;
  ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);
  return nullptr;
}

}  // namespace Marbas
