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

class Semahore {};

class Fence {};

/**
 * barrier
 */

enum class ResourceUsage {
  READ,
  WRITE,
  TRANSFER_SRC,
  TRANSFER_DST,
  PRESENT,
};

struct BufferBarrier {
  ResourceUsage srcUsage = ResourceUsage::READ;
  ResourceUsage dstUsage = ResourceUsage::WRITE;
  Buffer& buffer;
};

struct ImageBarrier {
  ResourceUsage srcUsage = ResourceUsage::TRANSFER_SRC;
  ResourceUsage dstUsage = ResourceUsage::TRANSFER_DST;
  Image& image;
};

struct ResourceBarrier {
  std::vector<BufferBarrier> bufferBarrier;
  std::vector<ImageBarrier> imageBarrier;
};

}  // namespace Marbas
