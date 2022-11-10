#pragma once

#include <vulkan/vulkan.hpp>

#include "ShaderModule.hpp"

namespace Marbas {

struct VulkanShaderModule final : public ShaderModule {
  vk::ShaderModule vkShaderModule;
};

}  // namespace Marbas
