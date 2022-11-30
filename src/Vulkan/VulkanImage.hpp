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

#include <vulkan/vulkan.hpp>

#include "Image.hpp"

namespace Marbas {

struct VulkanImage final : public Image {
  vk::Image vkImage;
  vk::DeviceMemory vkImageMemory;

  vk::Buffer vkStagingBuffer;
  vk::DeviceMemory vkStagingBufferMemory;
  vk::ImageAspectFlags vkAspect;
};

struct VulkanImageView final : public ImageView {
  vk::ImageView vkImageView;
};

}  // namespace Marbas
