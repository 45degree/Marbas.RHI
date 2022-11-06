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

namespace Marbas {

class VulkanPipelineContext final : public PipelineContext {
 public:
  Pipeline*
  CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) override;

  void
  DestroyPipeline(Pipeline* pipeline) override;

 public:
  Sampler*
  CreateSampler() override;

  void
  DestroySampler(Sampler* sampler) override;

 public:
  DescriptorSet*
  CreateDescriptorSet() override;

  void
  DestroyDescriptorSet(DescriptorSet* descriptorSet) override;

  void
  BindImage(DescriptorSet* descriptorSet, uint16_t bindingPoint, ImageView* imageView, Sampler* sampler) override;

  void
  BindBuffer(DescriptorSet* descriptorSet, uint16_t bindingPoint, Buffer* buffer, uint32_t offset) override;

  DescriptorPool*
  CreateDescriptorPool() override;

  void
  DestroyDescriptorPool(DescriptorPool* descriptorPool) override;

 private:
  vk::Device m_device;
};

}  // namespace Marbas
