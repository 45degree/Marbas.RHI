#pragma once

#include <vulkan/vulkan.hpp>

#include "Image.hpp"

namespace Marbas {

struct VulkanImage final : public Image {
  vk::Image vkImage;
  vk::DeviceMemory vkImageMemory;

  vk::Buffer vkStagingBuffer;
  vk::DeviceMemory vkStagingBufferMemory;

  vk::ImageLayout currentLayout;  // WARNING: render pass or command will change it
  vk::ImageAspectFlags aspect;
};

struct VulkanImageView final : public ImageView {
  vk::ImageView vkImageView;
};

}  // namespace Marbas
