/**
 * Copyright 2022.11.5 45degree
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

#include "VulkanImageContext.hpp"

#include "VulkanImage.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

Image*
VulkanImageContext::CreateImage(const ImageCreateInfo& imageCreateInfo) {
  vk::ImageCreateInfo createInfo;
  uint32_t width, height, depth;

  auto* vulkanImage = new VulkanImage();
  vulkanImage->depth = depth;
  vulkanImage->width = width;
  vulkanImage->height = height;
  vulkanImage->usage = imageCreateInfo.usage;
  vulkanImage->currentLayout = vk::ImageLayout::eUndefined;
  vulkanImage->mipMapLevel = createInfo.mipLevels;
  vulkanImage->format = imageCreateInfo.format;

  if (imageCreateInfo.usage & ImageUsageFlags::DEPTH) {
    vulkanImage->aspect = vk::ImageAspectFlagBits::eDepth;
  }
  if (imageCreateInfo.usage & ImageUsageFlags::SHADER_READ ||
      imageCreateInfo.usage & ImageUsageFlags::RENDER_TARGET ||
      imageCreateInfo.usage & ImageUsageFlags::PRESENT) {
    vulkanImage->aspect = vk::ImageAspectFlagBits::eDepth;
  }

  createInfo.setFormat(ConvertToVulkanFormat(imageCreateInfo.format));
  createInfo.setExtent(vk::Extent3D(width, height, depth));
  createInfo.setInitialLayout(vk::ImageLayout::eUndefined);

  // TODO:
  createInfo.setTiling(vk::ImageTiling::eLinear);
  createInfo.setSamples(vk::SampleCountFlagBits::e1);

  // clang-format off
  std::visit([&](auto&& imageDesc) {
    using T = std::decay_t<decltype(imageDesc)>;
    if constexpr (std::is_same_v<T, Image2DDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(1);
      vulkanImage->arrayLayer = 1;
    } else if constexpr (std::is_same_v<T, Image2DArrayDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(imageDesc.arraySize);
      vulkanImage->arrayLayer = imageDesc.arraySize;
    } else if constexpr (std::is_same_v<T, CubeMapImageDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(6);
      createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
      vulkanImage->arrayLayer = 6;
    } else if constexpr (std::is_same_v<T, CubeMapArrayImageDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(6 * imageDesc.arraySize);
      createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
      vulkanImage->arrayLayer = 6 * imageDesc.arraySize;
    }
  },imageCreateInfo.imageDesc);
  // clang-format on

  createInfo.setUsage(ConvertToVulkanImageUsage(imageCreateInfo.usage));
  createInfo.setSharingMode(vk::SharingMode::eExclusive);

  vulkanImage->image = m_device.createImage(createInfo);

  return vulkanImage;
}

void
VulkanImageContext::DestroyImage(Image* image) {
  auto* vulkanImage = static_cast<VulkanImage*>(image);
  m_device.destroyImage(vulkanImage->image);

  delete image;
}

}  // namespace Marbas
