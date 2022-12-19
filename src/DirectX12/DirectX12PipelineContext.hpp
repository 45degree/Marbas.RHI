/**
 * Copyright 2022.12.19 45degree
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

#include "DirectX12Common.hpp"
#include "PipelineContext.hpp"

namespace Marbas {

class DirectX12PipelineContext final : public PipelineContext {
 public:
  DirectX12PipelineContext() = default;
  ~DirectX12PipelineContext() override = default;

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
  ShaderModule*
  CreateShaderModule(std::span<char> sprivCode) override;

  void
  DestroyShaderModule(ShaderModule* shaderModule) override;

 public:
  DescriptorSetLayout*
  CreateDescriptorSetLayout(std::span<DescriptorSetLayoutBinding> layoutBinding) override;

  void
  DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout) override;

  DescriptorSet*
  CreateDescriptorSet(const DescriptorPool* pool, const DescriptorSetLayout* layout) override;

  void
  DestroyDescriptorSet(const DescriptorPool* pool, DescriptorSet* descriptorSet) override;

  void
  BindImage(const BindImageInfo& bindImageInfo) override;

  void
  BindBuffer(const BindBufferInfo& bindBufferInfo) override;

  DescriptorPool*
  CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) override;

  void
  DestroyDescriptorPool(DescriptorPool* descriptorPool) override;

 public:
  FrameBuffer*
  CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) override;

  void
  DestroyFrameBuffer(FrameBuffer* frameBuffer) override;

 private:
  ID3D12Device* m_device;
};

}  // namespace Marbas
