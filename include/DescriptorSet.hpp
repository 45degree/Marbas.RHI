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

#include <cstdint>
#include <vector>

#include "Buffer.hpp"
#include "Image.hpp"
#include "Sampler.hpp"

namespace Marbas {

enum class DescriptorType {
  UNIFORM_BUFFER,
  DYNAMIC_UNIFORM_BUFFER,
  IMAGE,
};

enum class DescriptorVisible {
  ALL,
  VERTEX_SHADER,
  FRAGMENT_SHADER,
};

struct DescriptorSetLayoutBinding {
  uint16_t bindingPoint = 0;
  DescriptorType descriptorType = DescriptorType::UNIFORM_BUFFER;
  uint32_t count = 1;
  DescriptorVisible visible = DescriptorVisible::ALL;
};

class DescriptorPool {};
class DescriptorSet {
 public:
  /**
   * @brief bind a image and sampler to the descriptor set
   *
   * @param bindingPoint binding point
   * @param imageView imageView
   * @param sampler sampler
   */
  virtual void
  BindImage(uint16_t bindingPoint, ImageView& imageView, Sampler& sampler) = 0;

  /**
   * @brief bind a unform buffer to the descriptor set
   *
   * @param bindingPoint binding point
   * @param buffer uniform buffer
   * @param offset the offset to the begin of the uniform buffer, this parameter is only worked when
   *        the binding point is bound by a dynamic uniform buffer.
   */
  virtual void
  BindUniformBuffer(uint16_t bindingPoint, Buffer* buffer, uint32_t offset) = 0;

  virtual void
  BindStorageBuffer(uint16_t bindingPoint, Buffer* buffer, uint32_t offset) = 0;

 private:
  std::vector<DescriptorSetLayoutBinding> m_descriptorLayout;
};

}  // namespace Marbas
