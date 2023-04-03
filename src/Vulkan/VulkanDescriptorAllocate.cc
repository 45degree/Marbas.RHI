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

#include "VulkanDescriptorAllocate.hpp"

namespace Marbas::RHI {

VulkanDescriptorAllocate::~VulkanDescriptorAllocate() {
  CleanUp();
  for (auto pool : m_freePools) {
    m_device.destroyDescriptorPool(pool);
  }
}

void
VulkanDescriptorAllocate::CleanUp() {
  // reset all used pools and add them to the free pools
  for (auto p : m_usedPools) {
    m_device.resetDescriptorPool(p);
    m_freePools.push_back(p);
  }

  // clear the used pools, since we've put them all in the free pools
  m_usedPools.clear();

  // reset the current pool handle back to null
  m_currentPool = VK_NULL_HANDLE;
}

vk::DescriptorSet
VulkanDescriptorAllocate::Allocate(const vk::DescriptorSetLayout& layout) {
  if (static_cast<VkDescriptorPool>(m_currentPool) == nullptr) {
    m_currentPool = GrabPool();
    m_usedPools.push_back(m_currentPool);
  }

  vk::DescriptorSetAllocateInfo allocInfo = {};
  allocInfo.setDescriptorSetCount(1);
  allocInfo.setDescriptorPool(m_currentPool);
  allocInfo.setSetLayouts(layout);

  vk::DescriptorSet set;
  vk::Result res = m_device.allocateDescriptorSets(&allocInfo, &set);

  if (res == vk::Result::eSuccess) {
    return set;
  }

  if (res != vk::Result::eErrorOutOfPoolMemory && res != vk::Result::eErrorFragmentedPool) {
    throw std::runtime_error("can't allocate descriptor set");
  }

  m_currentPool = GrabPool();
  m_usedPools.push_back(m_currentPool);
  res = m_device.allocateDescriptorSets(&allocInfo, &set);
  if (res == vk::Result::eSuccess) {
    return set;
  }
  throw std::runtime_error("can't allocate descriptor set");
}

vk::DescriptorPool
VulkanDescriptorAllocate::CreatePool(uint32_t count) {
  PoolSize poolSizes;

  std::vector<vk::DescriptorPoolSize> sizes;

  for (auto sz : poolSizes.sizes) {
    sizes.emplace_back(sz.first, static_cast<uint32_t>(sz.second * count));
  }

  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setMaxSets(count);
  createInfo.setPoolSizes(sizes);

  return m_device.createDescriptorPool(createInfo);
}

vk::DescriptorPool
VulkanDescriptorAllocate::GrabPool() {
  if (!m_freePools.empty()) {
    auto pool = m_freePools.back();
    m_freePools.pop_back();
    return pool;
  } else {
    return CreatePool(1000);
  }
}

}  // namespace Marbas::RHI
