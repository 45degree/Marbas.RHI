/**
 * Copyright 2022.10.31 45degree
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
#include <absl/container/flat_hash_set.h>

#include <vulkan/vulkan.hpp>

#include "PipelineContext.hpp"
#include "VulkanDescriptorAllocate.hpp"
#include "VulkanPipeline.hpp"

template <>
struct std::hash<Marbas::DescriptorSetArgument> {
  size_t
  operator()(const Marbas::DescriptorSetArgument& argument) const {
    absl::Hash<std::map<uint16_t, Marbas::DescriptorType>> hash;
    return hash(argument.m_bindingInfo);
  }
};

template <>
struct std::hash<vk::Sampler> {
  size_t
  operator()(const vk::Sampler& sampler) const {
    return absl::Hash<VkSampler>()(static_cast<VkSampler>(sampler));
  }
};

template <>
struct std::hash<vk::Pipeline> {
  size_t
  operator()(const vk::Pipeline& sampler) const {
    return absl::Hash<VkPipeline>()(static_cast<VkPipeline>(sampler));
  }
};

template <>
struct std::hash<vk::DescriptorSet> {
  size_t
  operator()(const vk::DescriptorSet& set) const {
    return absl::Hash<VkDescriptorSet>()(static_cast<VkDescriptorSet>(set));
  }
};

template <>
struct std::hash<vk::PipelineLayout> {
  size_t
  operator()(const vk::PipelineLayout& layout) const {
    return absl::Hash<VkPipelineLayout>()(static_cast<VkPipelineLayout>(layout));
  }
};

template <>
struct std::hash<vk::DescriptorSetLayout> {
  size_t
  operator()(const vk::DescriptorSetLayout& layout) const {
    return absl::Hash<VkDescriptorSetLayout>()(static_cast<VkDescriptorSetLayout>(layout));
  }
};

namespace Marbas {

struct VulkanPipelineLayoutArgument {
  std::vector<vk::DescriptorSetLayout> sets;
  uint32_t size;

  bool
  operator==(const VulkanPipelineLayoutArgument& argument) const {
    return sets == argument.sets && size == argument.size;
  }
};

}  // namespace Marbas

template <>
struct std::hash<Marbas::VulkanPipelineLayoutArgument> {
  size_t
  operator()(const Marbas::VulkanPipelineLayoutArgument& argument) const {
    return absl::Hash<std::vector<vk::DescriptorSetLayout>>()(argument.sets) + absl::Hash<uint32_t>()(argument.size);
  }
};

namespace Marbas {

class VulkanPipelineContext final : public PipelineContext {
  friend class VulkanGraphicsCommandBuffer;
  friend class VulkanComputeCommandBuffer;

 public:
  explicit VulkanPipelineContext(vk::Device device, vk::PhysicalDevice physicalDevice)
      : PipelineContext(), m_device(device), m_descriptorAllocate(device), m_physicalDevice(physicalDevice) {}
  ~VulkanPipelineContext() override {
    // destroy pipeline
    for (auto [pipeline, pipelineInfo] : m_computePipeline) {
      DestroyPipeline(reinterpret_cast<uintptr_t>(static_cast<VkPipeline>(pipeline)));
    }
    for (auto [pipeline, pipelineInfo] : m_graphicsPipeline) {
      DestroyPipeline(reinterpret_cast<uintptr_t>(static_cast<VkPipeline>(pipeline)));
    }

    m_descriptorAllocate.CleanUp();

    for (auto&& [argument, setLayout] : m_descriptorSetLayoutCache) {
      m_device.destroyDescriptorSetLayout(setLayout);
    }

    for (auto&& [argument, pipelineLayout] : m_pipelineLayoutCache) {
      m_device.destroyPipelineLayout(pipelineLayout);
    }
  }

 public:
  uintptr_t
  CreatePipeline(const GraphicsPipeLineCreateInfo& createInfo) override;

  uintptr_t
  CreatePipeline(const ComputePipelineCreateInfo& createInfo) override;

  void
  DestroyPipeline(uintptr_t pipeline) override;

 public:
  uintptr_t
  CreateSampler(const SamplerCreateInfo& createInfo) override;

  void
  DestroySampler(uintptr_t sampler) override;

 public:
  uintptr_t
  CreateDescriptorSet(const DescriptorSetArgument& argument) override;

  void
  DestroyDescriptorSet(uintptr_t descriptorSet) override;

  void
  BindImage(const BindImageInfo& bindImageInfo) override;

  void
  BindBuffer(const BindBufferInfo& bindBufferInfo) override;

  void
  BindStorageImage(const BindStorageImageInfo& BindStorageImageInfo) override;

 public:
  FrameBuffer*
  CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) override;

  void
  DestroyFrameBuffer(FrameBuffer* frameBuffer) override;

 private:
  vk::RenderPass
  CreateRenderPass(const RenderTargetDesc& renderTargetDesc, const vk::PipelineBindPoint& pipelineType);

  vk::ShaderModule
  CreateShaderModule(const std::vector<char>& spirvCode, ShaderType type);

  vk::PipelineLayout
  CreatePipelineLayout(const VulkanPipelineLayoutArgument& argument);

  void
  DestroyPipelineLayout(const vk::PipelineLayout& layout);

  vk::DescriptorSetLayout
  CreateDescriptorSetLayout(const DescriptorSetArgument& argument);

  void
  DestroyDescriptorSetLayout(const vk::DescriptorSetLayout& layout);

 private:
  vk::Device m_device;
  vk::PhysicalDevice m_physicalDevice;

  RHI::VulkanDescriptorAllocate m_descriptorAllocate;

  absl::flat_hash_set<vk::Sampler> m_samplers;
  absl::flat_hash_map<vk::Pipeline, VulkanGraphicsPipelineInfo> m_graphicsPipeline;
  absl::flat_hash_map<vk::Pipeline, VulkanComputePipelineInfo> m_computePipeline;
  absl::flat_hash_map<vk::DescriptorSet, vk::DescriptorSetLayout> m_descriptorSets;

  // cache
  absl::flat_hash_map<vk::DescriptorSetLayout, int> m_descriptorSetLayoutCount;
  absl::flat_hash_map<DescriptorSetArgument, vk::DescriptorSetLayout> m_descriptorSetLayoutCache;
  absl::flat_hash_map<vk::PipelineLayout, int> m_pipelineLayoutCount;
  absl::flat_hash_map<VulkanPipelineLayoutArgument, vk::PipelineLayout> m_pipelineLayoutCache;
};

}  // namespace Marbas
