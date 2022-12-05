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

#include <span>

#include "DescriptorSet.hpp"
#include "Pipeline.hpp"

namespace Marbas {

struct BindBufferInfo final {
  DescriptorSet* descriptorSet;
  DescriptorType descriptorType;
  uint16_t bindingPoint;
  Buffer* buffer;
  uint32_t offset;
  uint32_t arrayElement;
};

struct BindImageInfo final {
  DescriptorSet* descriptorSet;
  uint16_t bindingPoint;
  ImageView* imageView;
  Sampler* sampler;
};

class PipelineContext {
 public:
  PipelineContext() = default;
  virtual ~PipelineContext() = default;

 public:
  virtual Pipeline*
  CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) = 0;

  virtual void
  DestroyPipeline(Pipeline* pipeline) = 0;

 public:
  virtual Sampler*
  CreateSampler(const SamplerCreateInfo& createInfo) = 0;

  virtual void
  DestroySampler(Sampler* sampler) = 0;

 public:
  virtual ShaderModule*
  CreateShaderModule(std::span<char> sprivCode) = 0;

  virtual void
  DestroyShaderModule(ShaderModule* shaderModule) = 0;

 public:
  virtual DescriptorSetLayout*
  CreateDescriptorSetLayout(std::span<DescriptorSetLayoutBinding> layoutBinding) = 0;

  virtual void
  DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout) = 0;

  virtual DescriptorSet*
  CreateDescriptorSet(const DescriptorPool* pool, const DescriptorSetLayout* layout) = 0;

  virtual void
  DestroyDescriptorSet(const DescriptorPool* pool, DescriptorSet* descriptorSet) = 0;

  virtual void
  BindImage(const BindImageInfo& bindImageInfo) = 0;

  virtual void
  BindBuffer(const BindBufferInfo& bindBufferInfo) = 0;

  virtual DescriptorPool*
  CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) = 0;

  virtual void
  DestroyDescriptorPool(DescriptorPool* descriptorPool) = 0;

 public:
  virtual FrameBuffer*
  CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) = 0;

  virtual void
  DestroyFrameBuffer(FrameBuffer* frameBuffer) = 0;
};

}  // namespace Marbas
