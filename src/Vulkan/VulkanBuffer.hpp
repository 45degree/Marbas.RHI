/**
 * Copyright 2022.11.7 45degree
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

#include <optional>
#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

namespace Marbas {

struct VulkanBuffer final : public Buffer {
  vk::Buffer vkBuffer;
  vk::DeviceMemory vkMemory;

  std::optional<vk::Buffer> stageBuffer = std::nullopt;
  std::optional<vk::DeviceMemory> stageBufferMemory = std::nullopt;
};

struct VulkanTexelBuffer final : public TexelBuffer {
  vk::BufferView vkBufferView;
};

}  // namespace Marbas
