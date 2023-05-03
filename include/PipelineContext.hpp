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
  uintptr_t descriptorSet;
  DescriptorType descriptorType;
  uint16_t bindingPoint;
  Buffer* buffer;
  uint32_t offset;
  uint32_t arrayElement;
};

struct BindImageInfo final {
  uintptr_t descriptorSet;
  uint16_t bindingPoint;
  ImageView* imageView;
  uintptr_t sampler;
};

struct BindStorageImageInfo final {
  uintptr_t descriptorSet;
  uint16_t bindingPoint;
  ImageView* imageView;
};

class PipelineContext {
 public:
  PipelineContext() = default;
  virtual ~PipelineContext() = default;

 public:
  virtual uintptr_t
  CreatePipeline(const GraphicsPipeLineCreateInfo& createInfo) = 0;

  virtual uintptr_t
  CreatePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

  virtual void
  DestroyPipeline(uintptr_t pipeline) = 0;

 public:
  virtual uintptr_t
  CreateSampler(const SamplerCreateInfo& createInfo) = 0;

  virtual void
  DestroySampler(uintptr_t sampler) = 0;

 public:
  virtual uintptr_t
  CreateDescriptorSet(const DescriptorSetArgument& arguement) = 0;

  virtual void
  DestroyDescriptorSet(uintptr_t descriptorSet) = 0;

  virtual void
  BindImage(const BindImageInfo& bindImageInfo) = 0;

  virtual void
  BindBuffer(const BindBufferInfo& bindBufferInfo) = 0;

  virtual void
  BindStorageImage(const BindStorageImageInfo& BindStorageImageInfo) = 0;

 public:
  virtual FrameBuffer*
  CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) = 0;

  virtual void
  DestroyFrameBuffer(FrameBuffer* frameBuffer) = 0;
};

}  // namespace Marbas
