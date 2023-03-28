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
  STORAGE_BUFFER,
};

struct DescriptorSetLayoutBinding {
  uint16_t bindingPoint = 0;
  DescriptorType descriptorType = DescriptorType::UNIFORM_BUFFER;
};

struct DescriptorSetLayout {};

/**
 * Descritor Pool
 */

struct DescriptorPoolSize {
  DescriptorType type;
  uint32_t size;
};

struct DescriptorPool {};
struct DescriptorSet {};

}  // namespace Marbas
