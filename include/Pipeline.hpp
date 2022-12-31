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
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "Buffer.hpp"
#include "DescriptorSet.hpp"
#include "Sampler.hpp"
#include "ShaderModule.hpp"

namespace Marbas {

enum class PipelineType {
  GRAPHICS,
  COMPUTE,
};

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
  PolygonMode polygonMode = PolygonMode::FILL;
  CullMode cullMode = CullMode::NONE;
  FrontFace frontFace = FrontFace::CCW;
  int depthBias = 0;
  float depthBiasClamp = 0;
  float slopeScaledDepthBias = 0;
  bool depthCilpEnable = false;
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
  bool depthBoundsTestEnable = false;
  bool stencilTestEnable = false;
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
  std::vector<BlendAttachment> attachments = {};
  std::array<float, 4> constances = {0, 0, 0, 0};
};

/**
 * view port state
 */

struct ViewportInfo {
  float x = 0;
  float y = 0;
  float width = 800;
  float height = 600;
  float minDepth = 0;
  float maxDepth = 1;
};

struct ScissorInfo {
  int32_t x = 0;
  int32_t y = 0;
  uint32_t width = 800;
  uint32_t height = 600;
};

/**
 * input assembly state create struct
 */

enum class PrimitiveTopology {
  POINT,
  LINE,
  TRIANGLE,
  PATCH,
};

/**
 * @class InputAssemblyStateCreateInfo
 * @brief Structure specifying parameters of a newly created pipeline input assembly state
 */
struct InputAssemblyStateCreateInfo {
  PrimitiveTopology topology = PrimitiveTopology::TRIANGLE;
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
  std::vector<char> code;
  std::string interName = "main";
};

/**
 * Multi Sample
 */

struct MultisampleCreateInfo {
  SampleCount rasterizationSamples = SampleCount::BIT1;
};

/**
 * graphics pipeline create info struct
 */

struct DepthTargetDesc {
  bool isClear;
  SampleCount sampleCount = SampleCount::BIT1;
};

struct ColorTargetDesc {
  bool isClear;
  bool isPresent;
  SampleCount sampleCount = SampleCount::BIT1;
  ImageFormat format = ImageFormat::RGBA;
};

struct ResolveTargetDesc {
  bool isPresent;
  SampleCount sampleCount = SampleCount::BIT1;
  ImageFormat format = ImageFormat::RGBA;
};

struct RenderTargetDesc {
  std::vector<ColorTargetDesc> colorAttachments;
  std::optional<DepthTargetDesc> depthAttachments = std::nullopt;
  std::vector<ResolveTargetDesc> resolveAttachments;
};

struct GraphicsPipeLineCreateInfo {
  InputLayoutDesc vertexInputLayout = {};
  DescriptorSetLayout* layout = nullptr;
  std::vector<ShaderStageCreateInfo> shaderStageCreateInfo = {};
  InputAssemblyStateCreateInfo inputAssemblyState = {};
  RasterizationCreateInfo rasterizationInfo = {};
  DepthStencilCreateInfo depthStencilInfo = {};
  MultisampleCreateInfo multisampleCreateInfo = {};
  ColorBlendCreateInfo blendInfo = {};
  RenderTargetDesc outputRenderTarget = {};
};

struct RasterizationDesc {
  PolygonMode polygonMode = PolygonMode::FILL;
  CullMode cullMode = CullMode::BACK;
  FrontFace frontFace = FrontFace::CCW;
  int depthBias = 0;
  float depthBiasClamp = 0;
  float slopeScaledDepthBias = 0;
  bool depthCilpEnable = true;
  bool multisampleEnable = false;
};

struct Pipeline {
  PipelineType pipelineType = PipelineType::GRAPHICS;
};

struct FrameBufferCreateInfo {
  uint32_t height = 800;
  uint32_t width = 600;
  uint32_t layer = 1;
  Pipeline* pieline = nullptr;
  struct Attachment {
    std::span<ImageView*> colorAttachments;
    ImageView* depthAttachment = nullptr;
    std::span<ImageView*> resolveAttachments;
  } attachments;
};
struct FrameBuffer {
  uint32_t height = 800;
  uint32_t width = 600;
  uint32_t layerCount = 1;
};

}  // namespace Marbas
