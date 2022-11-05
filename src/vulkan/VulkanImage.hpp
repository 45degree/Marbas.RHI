#pragma once

#include <vulkan/vulkan.hpp>

#include "Image.hpp"

namespace Marbas {

struct VulkanImageData {
  uint32_t arrayLayer = 1;
  vk::Image image;
  vk::ImageLayout currentLayout;  // WARNING: render pass or command will change it
  vk::ImageAspectFlags aspect;
};

}  // namespace Marbas
