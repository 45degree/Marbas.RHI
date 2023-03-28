/**
 * Copyright 2022.11.2 45degree
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

#include <vector>

#include "Buffer.hpp"
#include "Image.hpp"

namespace Marbas {

struct Semaphore {};

struct Fence {};

/**
 * barrier
 */

struct BufferBarrier {
  uint32_t waitUsage = BufferUsageFlags::READ;
  uint32_t dstUsage = BufferUsageFlags::WRITE;
  Buffer* buffer;
};

struct ImageBarrier {
  Image* image;
};

}  // namespace Marbas
