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

#include <vulkan/vulkan.hpp>

#include "PipelineContext.hpp"
#include "VulkanDescriptor.hpp"
#include "VulkanPipeline.hpp"

namespace Marbas {

class VulkanPipelineContext final : public PipelineContext {
 public:
  explicit VulkanPipelineContext(vk::Device device) : PipelineContext(), m_device(device) {}

 public:
  Pipeline*
  CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) override;

  void
  DestroyPipeline(Pipeline* pipeline) override;

 public:
  Sampler*
  CreateSampler(const SamplerCreateInfo& createInfo) override;

  void
  DestroySampler(Sampler* sampler) override;

 public:
  DescriptorSetLayout*
  CreateDescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding>& layoutBinding) override;

  void
  DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout) override;

  DescriptorSet*
  CreateDescriptorSet(const DescriptorPool* pool, const DescriptorSetLayout* descriptorLayout) override;

  void
  DestroyDescriptorSet(const DescriptorPool* pool, DescriptorSet* descriptorSet) override;

  void
  BindImage(const BindImageInfo& bindImageInfo) override;

  void
  BindBuffer(const BindBufferInfo& bindBufferInfo) override;

  DescriptorPool*
  CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize) override;

  void
  DestroyDescriptorPool(DescriptorPool* descriptorPool) override;

 private:
  vk::RenderPass
  CreateRenderPass(const RenderTargetDesc& renderTargetDesc, const vk::PipelineBindPoint& pipelineType);

  vk::PipelineLayout
  CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptorSetLayout);

  vk::ShaderModule
  CreateShaderModule(const std::vector<char>& spirvCode, ShaderType type);

 public:
  FrameBuffer*
  CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) override;

  void
  DestroyFrameBuffer(FrameBuffer* frameBuffer) override;

 private:
  vk::Device m_device;
};

}  // namespace Marbas
