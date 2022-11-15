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

#include <array>
#include <cstdint>
#include <vector>

namespace Marbas {

enum class ElementType {
  R32_SFLOAT,           // float
  R32G32_SFLOAT,        // vec2
  R32G32B32_SFLOAT,     // vec3
  R32G32B32A32_SFLOAT,  // vec4
  R32G32_SINT,          // ivec2
  R32G32B32A32_UINT,    // uvec4
  R64_SFLOAT,           // double
};

enum class VertexInputClass {
  VERTEX = 0,
  INSTANCE = 1,
};

struct InputElementDesc {
  int binding = 0;
  ElementType format = ElementType::R32G32B32_SFLOAT;
  uint32_t attribute = 0;
  uint32_t offset = 0;
  uint32_t instanceStepRate = 0;
};

struct InputElementView {
  int binding = 0;
  uint32_t stride = 0;
  VertexInputClass inputClass = VertexInputClass::VERTEX;
};

struct InputLayoutDesc {
  std::vector<InputElementDesc> elementDesc = {};
  std::vector<InputElementView> viewDesc = {};
};

enum class BufferType {
  VERTEX_BUFFER,
  INDEX_BUFFER,
  UNIFORM_BUFFER,
  STORAGE_BUFFER,
};

struct BufferUsageFlags {
  enum value : uint32_t {
    TRANSFER_SRC = 1,
    TRANSFER_DST = 1 << 2,
    READ = 1 << 3,
    WRITE = 1 << 4,
  };
};

struct Buffer {
  BufferType bufferType = BufferType::VERTEX_BUFFER;
  uint32_t size = 0;
};

}  // namespace Marbas
