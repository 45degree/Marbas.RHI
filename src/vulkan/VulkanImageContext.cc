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

void
VulkanImageContext::CreateImage(Image& image) {
  vk::ImageCreateInfo createInfo;
  createInfo.setFormat(ConvertToVulkanFormat(image.format));
  createInfo.setExtent(vk::Extent3D(image.width, image.height, image.depth));
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
    } else if constexpr (std::is_same_v<T, Image2DArrayDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(imageDesc.arraySize);
    } else if constexpr (std::is_same_v<T, CubeMapImageDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(6);
      createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    } else if constexpr (std::is_same_v<T, CubeMapArrayImageDesc>) {
      createInfo.setImageType(vk::ImageType::e2D);
      createInfo.setArrayLayers(6 * imageDesc.arraySize);
      createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    }
  },image.imageDesc);
  // clang-format on
  createInfo.setUsage();
  createInfo.setSharingMode();

  image.vulkanData->currentLayout = vk::ImageLayout::eUndefined;
  image.vulkanData->image = m_device.createImage(createInfo);
}

}  // namespace Marbas
