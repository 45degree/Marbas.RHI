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
#include <string>
#include <vector>

#include "Buffer.hpp"
#include "Sampler.hpp"
#include "ShaderModule.hpp"

namespace Marbas {

/**
 * Rasterization Create Info
 */

enum class PolygonMode {
  FILL,
  POINT,
  LINE,
};

enum class CullMode {
  NONE,
  FRONT,
  BACK,
  FRONT_AND_BACK,
};

enum class FrontFace {
  CW,
  CCW,
};

struct RasterizationCreateInfo {
  bool depthClampEnable = false;
  bool rasterizerDiscardEnable = false;
  PolygonMode polygonMode = PolygonMode::FILL;
  float lineWidth = 1.0f;
  CullMode cullMode = CullMode::NONE;
  FrontFace frontFace = FrontFace::CCW;
  bool depthBiasEnable = false;
  float depthBiasConstantFactor = 0.0f;
  float depthBiasClamp = 0.0f;
  float depthBiasSlopeFactor = 0.0f;
};

/**
 * depth stencil create info
 */

enum class DepthCompareOp {
  ALWAYS,
  NEVER,
  LESS,
  EQUAL,
  LEQUAL,
  GREATER,
  NOTEQUAL,
  GEQUAL,
};

struct DepthStencilCreateInfo {
  bool depthTestEnable = true;
  bool depthWriteEnable = true;
  DepthCompareOp depthCompareOp = DepthCompareOp::LESS;
  bool depthBoundsTestEnable = true;
  bool stencilTestEnable = true;
};

/**
 * color blend create info
 */

enum class BlendOp {
  ADD,               // result = src + dst
  SUBTRACT,          // result = src - dst
  REVERSE_SUBTRACT,  // resutl = dst - src
};

enum class LogicOp {
  CLEAR,          // 0
  SET,            // 1
  COPY,           // S
  COPY_INVERTED,  //~S
  NOOP,           // D
  INVERT,         // ~D
  AND,            // S & D
  NAND,           // ~(S & D)
  OR,             // S | D
  NOR,            // ~(S | D)
  XOR,            // S ^ D
  EQUIV,          // ~(S ^ D)
  AND_REVERSE,    // S & ~D
  AND_INVERTED,   // ~S & D
  OR_REVERSE,     // S | ~D
  OR_INVERTED,    // ~S | D
};

enum class BlendFactor {
  ZERO,                      // factor equal 0
  ONE,                       // factor equal 1
  SRC_COLOR,                 // factor equal src
  ONE_MINUS_SRC_COLOR,       // factor equal 1 - src
  DST_COLOR,                 // factor equal dst
  ONE_MINUS_DST_COLOR,       // factor equal 1 − dst
  SRC_ALPHA,                 // factor equal alpha's value of src
  ONE_MINUS_SRC_ALPHA,       // factor equal 1 - alpha's value of src
  DST_ALPHA,                 // factor equal alpha's value of dst
  ONE_MINUS_DST_ALPHA,       // factor equal 1 - alpha's value of dst
  CONSTANT_COLOR,            // factor equal a const
  ONE_MINUS_CONSTANT_COLOR,  // factor equal 1 - the const vaule
  CONSTANT_ALPHA,            // factor equal alpha's value of the const
  ONE_MINUS_CONSTANT_ALPHA,  // factor equal 1 - alpha's value of the const
};

struct BlendAttachment {
  bool blendEnable = false;
  BlendFactor srcColorBlendFactor = BlendFactor::SRC_ALPHA;
  BlendFactor dstColorBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;
  BlendOp colorBlendOp = BlendOp::ADD;
  BlendFactor srcAlphaBlendFactor = BlendFactor::SRC_ALPHA;
  BlendFactor dstAlphaBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;
  BlendOp alphaBlendOp = BlendOp::ADD;
};

struct ColorBlendCreateInfo {
  bool logicOpEnable = false;
  LogicOp logicOp = LogicOp::COPY;
  std::vector<BlendAttachment> attachments = {};
  std::array<float, 4> constances = {0, 0, 0, 0};
};

/**
 * view port state
 */

struct ViewportInfo {
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t width = 800;
  uint32_t height = 600;
  uint32_t minDepth = 0;
  uint32_t maxDepth = 1;
};

struct ScissorInfo {
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t width = 800;
  uint32_t height = 600;
};

struct ViewportStateCreateInfo {
  std::vector<ViewportInfo> viewportInfos = {};
  std::vector<ScissorInfo> scissorInfo = {};
};

/**
 * input assembly state create struct
 */

enum class PrimitiveTopology {
  POINT,
  LINE,
  LINE_STRIP,
  TRIANGLE,
  TRIANGLE_STRIP,
  TRIANGLE_FAN,
  LINE_WITH_ADJACENCY,
  LINE_STRIP_WITH_ADJACENCY,
  TRIANGLE_WITH_ADJACENCY,
  TRIANGLE_STRIP_WITH_ADJACENCY,
  PATCH,
};

/**
 * @class InputAssemblyStateCreateInfo
 * @brief Structure specifying parameters of a newly created pipeline input assembly state
 */
struct InputAssemblyStateCreateInfo {
  PrimitiveTopology topology = PrimitiveTopology::LINE;
  bool primitiveRestartEnable = false;
};

/**
 * shader staae create struct
 */

enum class ShaderType {
  VERTEX_SHADER,
  FRAGMENT_SHADER,
  GEOMETRY_SHADER,
};

struct ShaderStageCreateInfo {
  ShaderType stage = ShaderType::VERTEX_SHADER;
  ShaderModule& shaderModule;
  std::string interName = "main";
};

/**
 * Multi Sample
 */

struct MultisampleCreateInfo {
  SampleCount rasterizationSamples = SampleCount::BIT1;
  bool sampleShadingEnable = true;
  float minSampleShading = 0;
  std::vector<uint32_t> sampleMask = {};
  bool alphaToCoverageEnable = true;
  bool alphaToOneEnable = true;
};

/**
 * graphics pipeline create info struct
 */

class RenderTarget {};
class RenderTargetDesc {
  bool isClear = true;
  bool isDepth = false;
};

class PipelineSignature;
struct GraphicsPipeLineCreateInfo {
  PipelineSignature* pipelineSignatrue;
  InputLayoutDesc vertexInputLayout = {};
  std::vector<ShaderStageCreateInfo> shaderStageCreateInfo = {};
  ViewportStateCreateInfo viewportStateCreateInfo = {};
  InputAssemblyStateCreateInfo inputAssemblyState = {};
  RasterizationCreateInfo rasterizationInfo = {};
  DepthStencilCreateInfo depthStencilInfo = {};
  MultisampleCreateInfo multisampleCreateInfo = {};
  ColorBlendCreateInfo blendInfo = {};
  std::vector<RenderTargetDesc> outputRenderTarget = {};
};

class PipelineSignature {};  // PSO
class Pipeline {};

}  // namespace Marbas
