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
