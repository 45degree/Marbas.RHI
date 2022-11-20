/**
 * Copyright 2022.11.1 45degree
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

namespace Marbas {

enum class SampleCount {
  BIT1,
  BIT2,
  BIT4,
  BIT8,
  BIT16,
  BIT32,
  BIT64,
};

// TODO: rename
enum class Filter {
  MIN_MAG_MIP_POINT,
  MIN_MAG_POINT_MIP_LINEAR,
  MIN_POINT_MAG_LINEAR_MIP_POINT,
  MIN_POINT_MAG_MIP_LINEAR,
  MIN_LINEAR_MAG_MIP_POINT,
  MIN_LINEAR_MAG_POINT_MIP_LINEAR,
  MIN_MAG_LINEAR_MIP_POINT,
  MIN_MAG_MIP_LINEAR,
  ANISOTROPIC,
  COMPARISON_MIN_MAG_MIP_POINT,
  COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
  COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
  COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
  COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
  COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
  COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
  COMPARISON_MIN_MAG_MIP_LINEAR,
  COMPARISON_ANISOTROPIC,
};

enum class BorderColor {
  FloatOpaqueBlack,
  FloatOpaqueWhite,
  FloatTransparentBlack,
  IntOpaqueBlack,
  IntOpaqueWhite,
  IntTransparentBlack,
};

enum class SamplerAddressMode {
  WRAP,
  MIRROR,
  CLAMP,
  BORDER,
  MIRROR_ONCE,
};

enum class ComparisonOp {
  NEVER,
  LESS,
  EQUAL,
  LESS_EQUAL,
  GREATER,
  NOT_EQUAL,
  GREATER_EQUAL,
  ALWAYS,
};

struct SamplerCreateInfo {
  Filter filter;
  SamplerAddressMode addressU;
  SamplerAddressMode addressV;
  SamplerAddressMode addressW;
  ComparisonOp comparisonOp;
  float mipLodBias = 0.0f;
  float minLod = 0.0f;
  float maxLod = 0.0f;
  float maxAnisotropy;
  BorderColor borderColor;
};

class Sampler {};

}  // namespace Marbas
