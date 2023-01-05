/**
 * Copyright 2022.11.30 45degree
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
#include "VulkanPipelineContext.hpp"

#include <optional>
#include <set>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanShaderModule.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

/**
 * @brief convert filter to vulkan used
 *
 * @param filter filter
 * @return { minFilter, magFilter, mipmapMode }
 */
FORCE_INLINE static void
SetFilterForSamplerCreateInfo(const Filter& filter, vk::SamplerCreateInfo& createInfo) {
  switch (filter) {
    case Filter::MIN_MAG_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      return;
    case Filter::MIN_MAG_POINT_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      return;
    case Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      return;
    case Filter::MIN_POINT_MAG_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      return;
    case Filter::MIN_LINEAR_MAG_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      return;
    case Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      return;
    case Filter::MIN_MAG_LINEAR_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      return;
    case Filter::MIN_MAG_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      return;
    case Filter::ANISOTROPIC:
      // TODO:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      createInfo.setAnisotropyEnable(true);
      return;
    case Filter::COMPARISON_MIN_MAG_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eNearest);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eNearest);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
      createInfo.setCompareEnable(true);
      return;
    case Filter::COMPARISON_ANISOTROPIC:
      // TODO:
      createInfo.setMinFilter(vk::Filter::eLinear);
      createInfo.setMagFilter(vk::Filter::eLinear);
      createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
      createInfo.setCompareEnable(true);
      createInfo.setAnisotropyEnable(true);
      return;
  }
}

FORCE_INLINE static vk::SamplerAddressMode
ConvertToVulkanSamplerAddressMode(const SamplerAddressMode& samplerAddressMode) {
  switch (samplerAddressMode) {
    case SamplerAddressMode::WRAP:
      return vk::SamplerAddressMode::eRepeat;
    case SamplerAddressMode::MIRROR:
      return vk::SamplerAddressMode::eMirroredRepeat;
    case SamplerAddressMode::CLAMP:
      return vk::SamplerAddressMode::eClampToEdge;
    case SamplerAddressMode::BORDER:
      return vk::SamplerAddressMode::eClampToBorder;
    case SamplerAddressMode::MIRROR_ONCE:
      return vk::SamplerAddressMode::eMirrorClampToEdge;
  }
}

static std::vector<uint32_t>
ConvertOpenGLSpirvToVulkanShader(const std::vector<uint32_t>& code, ShaderType shaderType) {
  spirv_cross::CompilerGLSL glsl(code);
  auto resource = glsl.get_shader_resources();

  // ubo
  for (const auto& ubo : resource.uniform_buffers) {
    glsl.set_decoration(ubo.id, spv::Decoration::DecorationDescriptorSet, 0);
  }

  // sampled image
  for (const auto& image : resource.sampled_images) {
    glsl.set_decoration(image.id, spv::Decoration::DecorationDescriptorSet, 1);
  }

  spirv_cross::CompilerGLSL::Options options;
  options.version = 450;
  options.es = false;
  options.vulkan_semantics = true;
  glsl.set_common_options(options);

  auto vulkanShaderSrc = glsl.compile();

  shaderc::Compiler compiler;
  shaderc::CompileOptions compileOptions;
  shaderc_shader_kind kind;
  switch (shaderType) {
    case ShaderType::VERTEX_SHADER:
      kind = shaderc_glsl_vertex_shader;
      break;
    case ShaderType::FRAGMENT_SHADER:
      kind = shaderc_fragment_shader;
      break;
    case ShaderType::GEOMETRY_SHADER:
      kind = shaderc_geometry_shader;
      break;
  }

  auto module = compiler.CompileGlslToSpv(vulkanShaderSrc, kind, "vulkan_shader_src", compileOptions);

  return {module.begin(), module.end()};
}

Sampler*
VulkanPipelineContext::CreateSampler(const SamplerCreateInfo& createInfo) {
  vk::SamplerCreateInfo vkCreateInfo;
  SetFilterForSamplerCreateInfo(createInfo.filter, vkCreateInfo);
  vkCreateInfo.setMaxAnisotropy(createInfo.maxAnisotropy);
  vkCreateInfo.setMinLod(createInfo.minLod);
  vkCreateInfo.setMaxLod(createInfo.maxLod);
  vkCreateInfo.setMipLodBias(createInfo.mipLodBias);
  vkCreateInfo.setUnnormalizedCoordinates(false);
  vkCreateInfo.setAddressModeU(ConvertToVulkanSamplerAddressMode(createInfo.addressU));
  vkCreateInfo.setAddressModeV(ConvertToVulkanSamplerAddressMode(createInfo.addressV));
  vkCreateInfo.setAddressModeW(ConvertToVulkanSamplerAddressMode(createInfo.addressW));

  switch (createInfo.borderColor) {
    case BorderColor::FloatOpaqueBlack:
      vkCreateInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
      break;
    case BorderColor::FloatOpaqueWhite:
      vkCreateInfo.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
      break;
    case BorderColor::FloatTransparentBlack:
      vkCreateInfo.setBorderColor(vk::BorderColor::eFloatTransparentBlack);
      break;
    case BorderColor::IntOpaqueBlack:
      vkCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
      break;
    case BorderColor::IntOpaqueWhite:
      vkCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueWhite);
      break;
    case BorderColor::IntTransparentBlack:
      vkCreateInfo.setBorderColor(vk::BorderColor::eIntTransparentBlack);
      break;
  }

  switch (createInfo.comparisonOp) {
    case ComparisonOp::NEVER:
      vkCreateInfo.setCompareOp(vk::CompareOp::eNever);
      break;
    case ComparisonOp::LESS:
      vkCreateInfo.setCompareOp(vk::CompareOp::eLess);
      break;
    case ComparisonOp::EQUAL:
      vkCreateInfo.setCompareOp(vk::CompareOp::eEqual);
      break;
    case ComparisonOp::LESS_EQUAL:
      vkCreateInfo.setCompareOp(vk::CompareOp::eLessOrEqual);
      break;
    case ComparisonOp::GREATER:
      vkCreateInfo.setCompareOp(vk::CompareOp::eGreater);
      break;
    case ComparisonOp::NOT_EQUAL:
      vkCreateInfo.setCompareOp(vk::CompareOp::eNotEqual);
      break;
    case ComparisonOp::GREATER_EQUAL:
      vkCreateInfo.setCompareOp(vk::CompareOp::eGreaterOrEqual);
      break;
    case ComparisonOp::ALWAYS:
      vkCreateInfo.setCompareOp(vk::CompareOp::eAlways);
      break;
  }

  auto sampler = m_device.createSampler(vkCreateInfo);

  auto* vulkanSampler = new VulkanSampler();
  vulkanSampler->vkSampler = sampler;

  return vulkanSampler;
}

void
VulkanPipelineContext::DestroySampler(Sampler* sampler) {
  if (sampler == nullptr) return;

  auto* vulkanSampler = static_cast<VulkanSampler*>(sampler);
  m_device.destroySampler(vulkanSampler->vkSampler);

  delete vulkanSampler;
}

vk::ShaderModule
VulkanPipelineContext::CreateShaderModule(const std::vector<char>& spirvCode, ShaderType type) {
  std::vector<uint32_t> code(std::ceil(spirvCode.size() / sizeof(uint32_t)));
  std::memcpy(code.data(), spirvCode.data(), spirvCode.size());
  auto newCode = ConvertOpenGLSpirvToVulkanShader(code, type);

  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCode(newCode);
  return m_device.createShaderModule(createInfo);
}

DescriptorSetLayout*
VulkanPipelineContext::CreateDescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding>& layoutBinding) {
  std::vector<vk::DescriptorSetLayoutBinding> uboBinding;
  std::vector<vk::DescriptorSetLayoutBinding> sampledImageBindind;
  for (const auto& binding : layoutBinding) {
    vk::DescriptorSetLayoutBinding vkLayoutBinding;
    vkLayoutBinding.setBinding(binding.bindingPoint);
    vkLayoutBinding.setDescriptorCount(1);
    vkLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eAll);

    switch (binding.descriptorType) {
      case DescriptorType::UNIFORM_BUFFER:
        vkLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        uboBinding.push_back(vkLayoutBinding);
        break;
      case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
        vkLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
        uboBinding.push_back(vkLayoutBinding);
        break;
      case DescriptorType::IMAGE:
        vkLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        sampledImageBindind.push_back(vkLayoutBinding);
        break;
    }
  }

  auto* vulkanDescriptorSetLayout = new VulkanDescriptorSetLayout();

  vk::DescriptorSetLayoutCreateInfo createInfo;
  createInfo.setBindings(uboBinding);
  vulkanDescriptorSetLayout->vkUboLayout = m_device.createDescriptorSetLayout(createInfo);

  createInfo.setBindings(sampledImageBindind);
  vulkanDescriptorSetLayout->vkSampledImageLayout = m_device.createDescriptorSetLayout(createInfo);

  return vulkanDescriptorSetLayout;
}

void
VulkanPipelineContext::DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout) {
  auto* vulkanDescriptorSetLayout = static_cast<VulkanDescriptorSetLayout*>(descriptorSetLayout);
  m_device.destroyDescriptorSetLayout(vulkanDescriptorSetLayout->vkUboLayout);
  m_device.destroyDescriptorSetLayout(vulkanDescriptorSetLayout->vkSampledImageLayout);
}

DescriptorPool*
VulkanPipelineContext::CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize) {
  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setMaxSets(200);

  std::vector<vk::DescriptorPoolSize> poolSizes;
  for (auto& descriptorInfo : descritorPoolSize) {
    vk::DescriptorPoolSize poolSize;
    poolSize.setDescriptorCount(descriptorInfo.size);
    poolSize.setType(ConvertToVulkanDescriptorType(descriptorInfo.type));

    poolSizes.push_back(poolSize);
  }
  createInfo.setPoolSizes(poolSizes);

  auto vkDescriptorPool = m_device.createDescriptorPool(createInfo);

  auto* descriptorPool = new VulkanDescriptorPool();
  descriptorPool->vkDescriptorPool = vkDescriptorPool;

  return descriptorPool;
}

void
VulkanPipelineContext::DestroyDescriptorPool(DescriptorPool* descriptorPool) {
  if (descriptorPool == nullptr) return;

  auto* vulkanDescriptorPool = static_cast<VulkanDescriptorPool*>(descriptorPool);

  m_device.destroyDescriptorPool(vulkanDescriptorPool->vkDescriptorPool);

  delete vulkanDescriptorPool;
}

DescriptorSet*
VulkanPipelineContext::CreateDescriptorSet(const DescriptorPool* pool, const DescriptorSetLayout* descriptorLayout) {
  auto* vulkanDescriptorSet = new VulkanDescriptorSet();
  auto* vulkanDescriptorSetLayout = static_cast<const VulkanDescriptorSetLayout*>(descriptorLayout);

  std::array descriptorLayouts = {vulkanDescriptorSetLayout->vkUboLayout,
                                  vulkanDescriptorSetLayout->vkSampledImageLayout};

  vk::DescriptorSetAllocateInfo allocateInfo;
  allocateInfo.setSetLayouts(descriptorLayouts);
  allocateInfo.setDescriptorPool(static_cast<const VulkanDescriptorPool*>(pool)->vkDescriptorPool);
  allocateInfo.setDescriptorSetCount(descriptorLayouts.size());

  auto vkDescriptorSets = m_device.allocateDescriptorSets(allocateInfo);
  DLOG_ASSERT(vkDescriptorSets.size() == 2);

  vulkanDescriptorSet->uniformBufferSet = vkDescriptorSets[0];
  vulkanDescriptorSet->sampledImageSet = vkDescriptorSets[1];

  return vulkanDescriptorSet;
}

void
VulkanPipelineContext::DestroyDescriptorSet(const DescriptorPool* descriptorPool, DescriptorSet* descriptorSet) {
  if (descriptorPool == nullptr || descriptorSet == nullptr) return;

  auto* vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet);
  delete vulkanDescriptorSet;
}

/**
 * pipeline
 */

// graphics pipeline
Pipeline*
VulkanPipelineContext::CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) {
  auto* graphicsPipeline = new VulkanPipeline();
  graphicsPipeline->vkRenderPass = CreateRenderPass(createInfo.outputRenderTarget, vk::PipelineBindPoint::eGraphics);

  // render pass
  vk::GraphicsPipelineCreateInfo vkCreateInfo;
  vkCreateInfo.setRenderPass(graphicsPipeline->vkRenderPass);

  // input state
  vk::PipelineVertexInputStateCreateInfo vkVertexInputStateCreateInfo;
  std::vector<vk::VertexInputAttributeDescription> vkInputAttributeDescs;
  std::vector<vk::VertexInputBindingDescription> vkInputBindingDescs;
  if (!createInfo.vertexInputLayout.elementDesc.empty()) {
    for (const auto& layout : createInfo.vertexInputLayout.elementDesc) {
      vk::VertexInputAttributeDescription vkInputAttributeDesc;
      vkInputAttributeDesc.setBinding(layout.binding);
      vkInputAttributeDesc.setLocation(layout.attribute);
      vkInputAttributeDesc.setOffset(layout.offset);
      vkInputAttributeDesc.setFormat(ConvertToVulkanFormat(layout.format));
      vkInputAttributeDescs.push_back(vkInputAttributeDesc);
    }
    for (const auto& viewDesc : createInfo.vertexInputLayout.viewDesc) {
      vk::VertexInputBindingDescription vkInputBindingDesc;
      vkInputBindingDesc.setBinding(viewDesc.binding);
      vkInputBindingDesc.setStride(viewDesc.stride);
      if (viewDesc.inputClass == VertexInputClass::VERTEX) {
        vkInputBindingDesc.setInputRate(vk::VertexInputRate::eVertex);
      } else {
        vkInputBindingDesc.setInputRate(vk::VertexInputRate::eInstance);
      }

      vkInputBindingDescs.push_back(vkInputBindingDesc);
    }
    vkVertexInputStateCreateInfo.setVertexAttributeDescriptions(vkInputAttributeDescs);
    vkVertexInputStateCreateInfo.setVertexBindingDescriptions(vkInputBindingDescs);
  }
  vkCreateInfo.setPVertexInputState(&vkVertexInputStateCreateInfo);

  // input assembly state
  vk::PipelineInputAssemblyStateCreateInfo vkInputAssemblyStateCreateInfo;
  vkInputAssemblyStateCreateInfo.setPrimitiveRestartEnable(false);
  vkInputAssemblyStateCreateInfo.setTopology(ConvertToVulkanTopology(createInfo.inputAssemblyState.topology));
  vkCreateInfo.setPInputAssemblyState(&vkInputAssemblyStateCreateInfo);

  // shader stage
  std::vector<vk::PipelineShaderStageCreateInfo> vkShaderStageCreateInfos;
  for (auto& shaderStageCreateInfo : createInfo.shaderStageCreateInfo) {
    vk::PipelineShaderStageCreateInfo info;

    // convert shader
    auto module = CreateShaderModule(shaderStageCreateInfo.code, shaderStageCreateInfo.stage);
    graphicsPipeline->shaderModule.push_back(module);
    info.setModule(module);
    if (shaderStageCreateInfo.stage == ShaderType::VERTEX_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eVertex);
    } else if (shaderStageCreateInfo.stage == ShaderType::FRAGMENT_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eFragment);
    } else if (shaderStageCreateInfo.stage == ShaderType::GEOMETRY_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eGeometry);
    }
    info.setPName(shaderStageCreateInfo.interName.c_str());
    vkShaderStageCreateInfos.push_back(info);
  }
  vkCreateInfo.setStages(vkShaderStageCreateInfos);

  // view port
  vk::PipelineViewportStateCreateInfo vkViewportStateCreateInfo;
  vkCreateInfo.setPViewportState(&vkViewportStateCreateInfo);

  // dynamic state
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo vkPipelineDynamicStateCreateInfo;
  vkPipelineDynamicStateCreateInfo.setDynamicStates(dynamicStates);
  vkCreateInfo.setPDynamicState(&vkPipelineDynamicStateCreateInfo);

  // resterization state
  vk::PipelineRasterizationStateCreateInfo vkRasterizationStateCreateInfo;
  vkRasterizationStateCreateInfo.setRasterizerDiscardEnable(false);
  vkRasterizationStateCreateInfo.setCullMode(ConvertToVulkanCullMode(createInfo.rasterizationInfo.cullMode));
  vkRasterizationStateCreateInfo.setLineWidth(1.0);
  vkRasterizationStateCreateInfo.setPolygonMode(ConvertToVulkanPolyMode(createInfo.rasterizationInfo.polygonMode));
  vkRasterizationStateCreateInfo.setFrontFace(ConvertToVulkanFrontFace(createInfo.rasterizationInfo.frontFace));

  vkRasterizationStateCreateInfo.setDepthClampEnable(true);
  vkRasterizationStateCreateInfo.setDepthBiasClamp(createInfo.rasterizationInfo.depthBiasClamp);
  vkRasterizationStateCreateInfo.setDepthBiasConstantFactor(0);
  vkRasterizationStateCreateInfo.setDepthBiasSlopeFactor(createInfo.rasterizationInfo.slopeScaledDepthBias);
  vkRasterizationStateCreateInfo.setDepthBiasEnable(false);

  vk::PipelineRasterizationDepthClipStateCreateInfoEXT vkDepthClipStateCreateInfo;
  vkDepthClipStateCreateInfo.setDepthClipEnable(createInfo.rasterizationInfo.depthCilpEnable);
  vkRasterizationStateCreateInfo.setPNext(&vkDepthClipStateCreateInfo);

  vkCreateInfo.setPRasterizationState(&vkRasterizationStateCreateInfo);

  // depth stecil
  vk::PipelineDepthStencilStateCreateInfo vkDepthStencilStateCreateInfo;
  vkDepthStencilStateCreateInfo.setDepthTestEnable(createInfo.depthStencilInfo.depthTestEnable);
  vkDepthStencilStateCreateInfo.setDepthWriteEnable(createInfo.depthStencilInfo.depthWriteEnable);
  vkDepthStencilStateCreateInfo.setDepthBoundsTestEnable(createInfo.depthStencilInfo.depthBoundsTestEnable);
  vkDepthStencilStateCreateInfo.setStencilTestEnable(createInfo.depthStencilInfo.stencilTestEnable);
  switch (createInfo.depthStencilInfo.depthCompareOp) {
    case DepthCompareOp::ALWAYS:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eAlways);
      break;
    case DepthCompareOp::NEVER:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eNever);
      break;
    case DepthCompareOp::LESS:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);
      break;
    case DepthCompareOp::EQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eEqual);
      break;
    case DepthCompareOp::LEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
      break;
    case DepthCompareOp::GREATER:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eGreater);
      break;
    case DepthCompareOp::NOTEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eNotEqual);
      break;
    case DepthCompareOp::GEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eGreaterOrEqual);
      break;
  }
  vkCreateInfo.setPDepthStencilState(&vkDepthStencilStateCreateInfo);

  // blend
  vk::PipelineColorBlendStateCreateInfo vkColorBlendStateCreateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> vkColorBlendAttachmentStates;
  vkColorBlendStateCreateInfo.setBlendConstants(createInfo.blendInfo.constances);
  for (auto& attachment : createInfo.blendInfo.attachments) {
    auto convertToVukanBlendOp = [](const BlendOp& blendOp) -> vk::BlendOp {
      if (blendOp == BlendOp::ADD) return vk::BlendOp::eAdd;
      if (blendOp == BlendOp::REVERSE_SUBTRACT) return vk::BlendOp::eReverseSubtract;
      if (blendOp == BlendOp::SUBTRACT) return vk::BlendOp::eSubtract;

      return vk::BlendOp::eAdd;
    };

    auto convertToVulkanFactory = [](const BlendFactor& factory) -> vk::BlendFactor {
      switch (factory) {
        case BlendFactor::ZERO:
          return vk::BlendFactor::eZero;
        case BlendFactor::ONE:
          return vk::BlendFactor::eOne;
        case BlendFactor::SRC_COLOR:
          return vk::BlendFactor::eSrcColor;
        case BlendFactor::ONE_MINUS_SRC_COLOR:
          return vk::BlendFactor::eOneMinusSrcColor;
        case BlendFactor::DST_COLOR:
          return vk::BlendFactor::eDstColor;
        case BlendFactor::ONE_MINUS_DST_COLOR:
          return vk::BlendFactor::eOneMinusDstColor;
        case BlendFactor::SRC_ALPHA:
          return vk::BlendFactor::eSrcAlpha;
        case BlendFactor::ONE_MINUS_SRC_ALPHA:
          return vk::BlendFactor::eOneMinusSrcAlpha;
        case BlendFactor::DST_ALPHA:
          return vk::BlendFactor::eDstAlpha;
        case BlendFactor::ONE_MINUS_DST_ALPHA:
          return vk::BlendFactor::eOneMinusDstAlpha;
        case BlendFactor::CONSTANT_COLOR:
          return vk::BlendFactor::eConstantColor;
        case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
          return vk::BlendFactor::eOneMinusConstantColor;
        case BlendFactor::CONSTANT_ALPHA:
          return vk::BlendFactor::eConstantAlpha;
        case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
          return vk::BlendFactor::eOneMinusConstantAlpha;
      }
    };

    auto writeMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG |
                     vk::ColorComponentFlagBits::eR;
    vk::PipelineColorBlendAttachmentState attachmentState;
    attachmentState.setBlendEnable(attachment.blendEnable);
    attachmentState.setColorBlendOp(convertToVukanBlendOp(attachment.colorBlendOp));
    attachmentState.setAlphaBlendOp(convertToVukanBlendOp(attachment.alphaBlendOp));
    attachmentState.setSrcColorBlendFactor(convertToVulkanFactory(attachment.srcColorBlendFactor));
    attachmentState.setSrcAlphaBlendFactor(convertToVulkanFactory(attachment.srcAlphaBlendFactor));
    attachmentState.setDstColorBlendFactor(convertToVulkanFactory(attachment.dstColorBlendFactor));
    attachmentState.setDstAlphaBlendFactor(convertToVulkanFactory(attachment.dstAlphaBlendFactor));
    attachmentState.setColorWriteMask(writeMask);

    vkColorBlendAttachmentStates.push_back(attachmentState);
  }
  vkColorBlendStateCreateInfo.setAttachments(vkColorBlendAttachmentStates);
  vkColorBlendStateCreateInfo.setLogicOpEnable(false);
  vkCreateInfo.setPColorBlendState(&vkColorBlendStateCreateInfo);

  // sample
  vk::PipelineMultisampleStateCreateInfo vkMultisampleStateCreateInfo;
  const auto& sampleCount = createInfo.multisampleCreateInfo.rasterizationSamples;
  vkMultisampleStateCreateInfo.setSampleShadingEnable(true);
  vkMultisampleStateCreateInfo.setMinSampleShading(0.2);
  vkMultisampleStateCreateInfo.setRasterizationSamples(ConvertToVulkanSampleCount(sampleCount));
  vkCreateInfo.setPMultisampleState(&vkMultisampleStateCreateInfo);

  // layout
  auto* vulkanDescriptorSetLayout = static_cast<VulkanDescriptorSetLayout*>(createInfo.layout);
  auto vkPipelineLayout = CreatePipelineLayout(vulkanDescriptorSetLayout);
  vkCreateInfo.setLayout(vkPipelineLayout);

  auto result = m_device.createGraphicsPipeline(nullptr, vkCreateInfo);
  if (result.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create the vulkan graphics pipepine");
  }
  graphicsPipeline->vkPipeline = result.value;
  graphicsPipeline->pipelineType = PipelineType::GRAPHICS;
  graphicsPipeline->vkPipelineLayout = vkPipelineLayout;
  return graphicsPipeline;
}

void
VulkanPipelineContext::DestroyPipeline(Pipeline* pipeline) {
  if (pipeline == nullptr) return;

  auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
  m_device.destroyRenderPass(vulkanPipeline->vkRenderPass);
  m_device.destroyPipeline(vulkanPipeline->vkPipeline);
  m_device.destroyPipelineLayout(vulkanPipeline->vkPipelineLayout);
  for (auto& module : vulkanPipeline->shaderModule) {
    m_device.destroyShaderModule(module);
  }

  delete vulkanPipeline;
}

FrameBuffer*
VulkanPipelineContext::CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) {
  auto* vulkanPipeline = static_cast<VulkanPipeline*>(createInfo.pieline);
  auto vkRenderPass = vulkanPipeline->vkRenderPass;

  vk::FramebufferCreateInfo vkFramebufferCreateInfo;
  std::vector<vk::ImageView> vkAttachments;
  std::transform(createInfo.attachments.colorAttachments.begin(), createInfo.attachments.colorAttachments.end(),
                 std::back_inserter(vkAttachments),
                 [](auto&& attachment) { return static_cast<VulkanImageView*>(attachment)->vkImageView; });
  if (createInfo.attachments.depthAttachment != nullptr) {
    vkAttachments.push_back(static_cast<VulkanImageView*>(createInfo.attachments.depthAttachment)->vkImageView);
  }
  std::transform(createInfo.attachments.resolveAttachments.begin(), createInfo.attachments.resolveAttachments.end(),
                 std::back_inserter(vkAttachments),
                 [](auto&& attachment) { return static_cast<VulkanImageView*>(attachment)->vkImageView; });

  vkFramebufferCreateInfo.setRenderPass(vkRenderPass);
  vkFramebufferCreateInfo.setAttachments(vkAttachments);
  vkFramebufferCreateInfo.setLayers(createInfo.layer);
  vkFramebufferCreateInfo.setHeight(createInfo.height);
  vkFramebufferCreateInfo.setWidth(createInfo.width);

  auto vkFramebuffer = m_device.createFramebuffer(vkFramebufferCreateInfo);

  auto* vulkanFrameBuffer = new VulkanFrameBuffer();
  vulkanFrameBuffer->vkFrameBuffer = vkFramebuffer;
  vulkanFrameBuffer->width = createInfo.width;
  vulkanFrameBuffer->height = createInfo.height;
  vulkanFrameBuffer->layerCount = createInfo.layer;

  return vulkanFrameBuffer;
}

void
VulkanPipelineContext::DestroyFrameBuffer(FrameBuffer* frameBuffer) {
  if (frameBuffer == nullptr) return;

  auto* vulkanFrameBuffer = static_cast<VulkanFrameBuffer*>(frameBuffer);
  m_device.destroyFramebuffer(vulkanFrameBuffer->vkFrameBuffer);

  delete vulkanFrameBuffer;
}

vk::RenderPass
VulkanPipelineContext::CreateRenderPass(const RenderTargetDesc& renderTargetDescs,
                                        const vk::PipelineBindPoint& pipelineType) {
  std::vector<vk::AttachmentDescription> attachmentDescriptions;
  std::vector<vk::AttachmentReference> colorAttachmentReferences;
  std::optional<vk::AttachmentReference> depthAttachmentReference;
  std::vector<vk::AttachmentReference> resolveAttachmentReference;

  for (auto colorTargetDesc : renderTargetDescs.colorAttachments) {
    vk::AttachmentDescription description;
    vk::AttachmentReference reference;
    reference.setAttachment(attachmentDescriptions.size());
    reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentReferences.push_back(reference);

    description.setFormat(ConvertToVulkanFormat(colorTargetDesc.format));
    description.setSamples(ConvertToVulkanSampleCount(colorTargetDesc.sampleCount));

    if (colorTargetDesc.isClear) {
      description.setInitialLayout(vk::ImageLayout::eUndefined);
      description.setLoadOp(vk::AttachmentLoadOp::eClear);
    } else {
      // TODO
      description.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
      description.setLoadOp(vk::AttachmentLoadOp::eLoad);
    }
    description.setStoreOp(vk::AttachmentStoreOp::eStore);

    if (colorTargetDesc.isPresent) {
      description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    } else {
      description.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }

    attachmentDescriptions.push_back(description);
  }

  // set depth attachment
  if (renderTargetDescs.depthAttachments.has_value()) {
    const auto& depthAttachment = *renderTargetDescs.depthAttachments;
    vk::AttachmentDescription description;
    depthAttachmentReference = vk::AttachmentReference();
    depthAttachmentReference->setAttachment(attachmentDescriptions.size());
    depthAttachmentReference->setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    description.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    description.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    description.setFormat(ConvertToVulkanFormat(ImageFormat::DEPTH));
    description.setSamples(ConvertToVulkanSampleCount(depthAttachment.sampleCount));

    if (depthAttachment.isClear) {
      description.setLoadOp(vk::AttachmentLoadOp::eClear);
    } else {
      description.setLoadOp(vk::AttachmentLoadOp::eLoad);
    }
    description.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachmentDescriptions.push_back(description);
  }

  // set resolve attachment
  for (const auto& resolveAttachment : renderTargetDescs.resolveAttachments) {
    vk::AttachmentDescription description;
    vk::AttachmentReference reference;
    reference.setAttachment(attachmentDescriptions.size());
    reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    resolveAttachmentReference.push_back(reference);

    description.setFormat(ConvertToVulkanFormat(resolveAttachment.format));
    description.setInitialLayout(vk::ImageLayout::eUndefined);
    description.setSamples(ConvertToVulkanSampleCount(resolveAttachment.sampleCount));
    description.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    description.setStoreOp(vk::AttachmentStoreOp::eStore);
    description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

    if (resolveAttachment.isPresent) {
      description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    } else {
      description.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }
    attachmentDescriptions.push_back(description);
  }

  vk::SubpassDescription vkSubpassDescription;
  vkSubpassDescription.setPipelineBindPoint(pipelineType);

  if (!colorAttachmentReferences.empty()) {
    vkSubpassDescription.setColorAttachments(colorAttachmentReferences);
  }
  if (!resolveAttachmentReference.empty()) {
    vkSubpassDescription.setResolveAttachments(resolveAttachmentReference);
  }
  if (depthAttachmentReference.has_value()) {
    vkSubpassDescription.setPDepthStencilAttachment(&depthAttachmentReference.value());
  }

  vk::RenderPassCreateInfo vkRenderPassCreateInfo;
  vkRenderPassCreateInfo.setAttachments(attachmentDescriptions);
  vkRenderPassCreateInfo.setSubpasses(vkSubpassDescription);

  return m_device.createRenderPass(vkRenderPassCreateInfo);
}

vk::PipelineLayout
VulkanPipelineContext::CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptorSetLayout) {
  vk::PipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
  if (descriptorSetLayout == nullptr) {
    return m_device.createPipelineLayout(vkPipelineLayoutCreateInfo);
  }

  std::array vkLayouts = {descriptorSetLayout->vkUboLayout, descriptorSetLayout->vkSampledImageLayout};
  vkPipelineLayoutCreateInfo.setSetLayouts(vkLayouts);
  return m_device.createPipelineLayout(vkPipelineLayoutCreateInfo);
}

void
VulkanPipelineContext::BindBuffer(const BindBufferInfo& bindBufferInfo) {
  auto* vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(bindBufferInfo.descriptorSet);
  auto* vulkanBuffer = static_cast<VulkanBuffer*>(bindBufferInfo.buffer);
  const auto& descriptorType = bindBufferInfo.descriptorType;

  vk::DescriptorBufferInfo vkDescriptorBufferInfo;
  vkDescriptorBufferInfo.setBuffer(vulkanBuffer->vkBuffer);
  vkDescriptorBufferInfo.setOffset(bindBufferInfo.offset);
  vkDescriptorBufferInfo.setRange(vulkanBuffer->size);

  vk::WriteDescriptorSet vkWriteDescriptorSet;
  vkWriteDescriptorSet.setDstSet(vulkanDescriptorSet->uniformBufferSet);
  vkWriteDescriptorSet.setDstBinding(bindBufferInfo.bindingPoint);
  vkWriteDescriptorSet.setDescriptorCount(1);
  vkWriteDescriptorSet.setBufferInfo(vkDescriptorBufferInfo);
  vkWriteDescriptorSet.setDstArrayElement(bindBufferInfo.arrayElement);

  if (descriptorType == DescriptorType::UNIFORM_BUFFER) {
    vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eUniformBuffer);
  } else if (descriptorType == DescriptorType::DYNAMIC_UNIFORM_BUFFER) {
    vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
  }
  m_device.updateDescriptorSets(vkWriteDescriptorSet, nullptr);
}

void
VulkanPipelineContext::BindImage(const BindImageInfo& bindImageInfo) {
  auto* vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(bindImageInfo.descriptorSet);
  auto* vulkanImageView = static_cast<VulkanImageView*>(bindImageInfo.imageView);
  auto* vulkanSampler = static_cast<VulkanSampler*>(bindImageInfo.sampler);

  const auto& vkImageView = vulkanImageView->vkImageView;
  const auto& vkSampler = vulkanSampler->vkSampler;

  vk::DescriptorImageInfo vkDescriptorImageInfo;
  vkDescriptorImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
  vkDescriptorImageInfo.setImageView(vkImageView);
  vkDescriptorImageInfo.setSampler(vkSampler);

  vk::WriteDescriptorSet vkWriteDescriptorSet;
  vkWriteDescriptorSet.setDstSet(vulkanDescriptorSet->sampledImageSet);
  vkWriteDescriptorSet.setDstBinding(bindImageInfo.bindingPoint);
  vkWriteDescriptorSet.setDescriptorCount(1);
  vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
  vkWriteDescriptorSet.setImageInfo(vkDescriptorImageInfo);
  vkWriteDescriptorSet.setDstArrayElement(0);

  m_device.updateDescriptorSets(vkWriteDescriptorSet, nullptr);
}
}  // namespace Marbas
