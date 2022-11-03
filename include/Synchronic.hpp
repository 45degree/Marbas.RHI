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

namespace Marbas {

class Semahore {};

class Fence {};

/**
 * barrier
 */

enum class PipelineState {
  TOP_OF_PIPE = 0x00000001,
  DRAW_INDIRECT = 0x00000002,
  VERTEX_INPUT = 0x00000004,
  VERTEX_SHADER = 0x00000008,
  TESSELLATION_CONTROL_SHADER = 0x00000010,
  TESSELLATION_EVALUATION_SHADER = 0x00000020,
  GEOMETRY_SHADER = 0x00000040,
  FRAGMENT_SHADER = 0x00000080,
  EARLY_FRAGMENT_TESTS = 0x00000100,
  LATE_FRAGMENT_TESTS = 0x00000200,
  COLOR_ATTACHMENT_OUTPUT = 0x00000400,
  COMPUTE_SHADER = 0x00000800,
  TRANSFER = 0x00001000,
  BOTTOM_OF_PIPE = 0x00002000,
  HOST = 0x00004000,
  ALL_GRAPHICS = 0x00008000,
  ALL_COMMANDS = 0x00010000,
};

class BufferBarrier {};

class ImageBarrier {};

class PipelineBarrier {
  PipelineState srcState;
  PipelineState dstState;
  std::vector<BufferBarrier> bufferBarrier;
  std::vector<ImageBarrier> imageBarrier;
};

}  // namespace Marbas
