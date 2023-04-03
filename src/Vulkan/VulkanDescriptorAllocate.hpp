/**
 * Copyright 2023.4.1 45degree
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

#include <absl/container/flat_hash_map.h>

#include <vulkan/vulkan.hpp>

namespace Marbas::RHI {

struct PoolSize {
  const absl::flat_hash_map<vk::DescriptorType, float> sizes = {
      {vk::DescriptorType::eSampler, 0.5f},
      {vk::DescriptorType::eCombinedImageSampler, 4.f},
      {vk::DescriptorType::eSampledImage, 4.f},
      {vk::DescriptorType::eStorageImage, 1.f},
      {vk::DescriptorType::eUniformTexelBuffer, 1.f},
      {vk::DescriptorType::eStorageTexelBuffer, 1.f},
      {vk::DescriptorType::eUniformBuffer, 2.f},
      {vk::DescriptorType::eStorageBuffer, 2.f},
      {vk::DescriptorType::eUniformBufferDynamic, 1.f},
      {vk::DescriptorType::eStorageBufferDynamic, 1.f},
      {vk::DescriptorType::eInputAttachment, 0.5f},
  };
};

class VulkanDescriptorAllocate final {
 public:
  explicit VulkanDescriptorAllocate(vk::Device device) : m_device(device) {}
  ~VulkanDescriptorAllocate();

  void
  CleanUp();

  vk::DescriptorSet
  Allocate(const vk::DescriptorSetLayout& layout);

  void
  Delete(const vk::DescriptorSet& set) {}

 private:
  vk::DescriptorPool
  CreatePool(uint32_t count);

  vk::DescriptorPool
  GrabPool();

 private:
  vk::Device m_device;
  vk::DescriptorPool m_currentPool;
  std::vector<vk::DescriptorPool> m_usedPools;
  std::vector<vk::DescriptorPool> m_freePools;
};

}  // namespace Marbas::RHI
