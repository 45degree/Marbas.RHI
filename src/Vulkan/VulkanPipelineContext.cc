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

#include <iostream>
#include <optional>
#include <set>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "VulkanBuffer.hpp"
#include "VulkanFormat.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
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

static void
SetAttachmentLayout(vk::AttachmentDescription& description, AttachmentInitAction initAction,
                    AttachmentFinalAction finalAction, uint32_t usage) {
  bool isColorTarget = usage & ImageUsageFlags::COLOR_RENDER_TARGET;
  bool isDepthStencil = usage & ImageUsageFlags::DEPTH_STENCIL;

  if (isColorTarget == isDepthStencil) {
    // never enter this
    LOG(ERROR) << "attachment usage must contain one of COLOR_RENDER_TARGET and DEPTH_STENCIL";
    throw std::runtime_error("attachment usage error");
  }

  if (finalAction == AttachmentFinalAction::PRESENT && isDepthStencil) {
    LOG(ERROR) << "the final action is present, but it used as depth stencil attachment";
    throw std::runtime_error("attachment usage error");
  }

  auto defaultImageLayout = GetDefaultImageLayoutFromUsage(usage);

  // set init layout
  bool isSample = usage & ImageUsageFlags::SHADER_READ;
  switch (initAction) {
    case AttachmentInitAction::CLEAR: {
      if (isColorTarget) {
        description.setLoadOp(vk::AttachmentLoadOp::eClear);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setInitialLayout(defaultImageLayout);
      } else if (isDepthStencil) {
        description.setLoadOp(vk::AttachmentLoadOp::eClear);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
        description.setInitialLayout(defaultImageLayout);
      }
      break;
    }
    case AttachmentInitAction::KEEP: {
      if (isColorTarget) {
        description.setLoadOp(vk::AttachmentLoadOp::eLoad);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setInitialLayout(defaultImageLayout);
      } else if (isDepthStencil) {
        description.setLoadOp(vk::AttachmentLoadOp::eLoad);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eLoad);
        description.setInitialLayout(defaultImageLayout);
      }
      break;
    }
    case AttachmentInitAction::DROP: {
      if (isColorTarget) {
        description.setLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setInitialLayout(defaultImageLayout);
      } else if (isDepthStencil) {
        description.setLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        description.setInitialLayout(vk::ImageLayout::eUndefined);
      }
      break;
    }
  }

  // set final layout
  switch (finalAction) {
    case AttachmentFinalAction::READ: {
      if (isColorTarget) {
        description.setStoreOp(vk::AttachmentStoreOp::eStore);
        description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
        description.setFinalLayout(defaultImageLayout);
      } else if (isDepthStencil) {
        description.setStoreOp(vk::AttachmentStoreOp::eStore);
        description.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
        description.setFinalLayout(defaultImageLayout);
      }
      break;
    }
    case AttachmentFinalAction::DISCARD: {
      description.setStoreOp(vk::AttachmentStoreOp::eDontCare);
      description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
      description.setFinalLayout(defaultImageLayout);
      break;
    }
    case AttachmentFinalAction::PRESENT: {
      description.setInitialLayout(vk::ImageLayout::eUndefined);
      description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
      description.setStoreOp(vk::AttachmentStoreOp::eStore);
      description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
      break;
    }
  }
}

uintptr_t
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
  m_samplers.insert(sampler);

  return reinterpret_cast<uintptr_t>(static_cast<VkSampler>(sampler));
}

void
VulkanPipelineContext::DestroySampler(uintptr_t sampler) {
  auto vkSampler = static_cast<vk::Sampler>(reinterpret_cast<VkSampler>(sampler));

  auto iter = m_samplers.find(vkSampler);
  if (iter == m_samplers.end()) return;

  m_samplers.erase(iter);
  m_device.destroySampler(vkSampler);
}

vk::ShaderModule
VulkanPipelineContext::CreateShaderModule(const std::vector<char>& spirvCode, ShaderType type) {
  std::vector<uint32_t> code(std::ceil(spirvCode.size() / sizeof(uint32_t)), 0);
  std::memcpy(code.data(), spirvCode.data(), spirvCode.size());

  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCode(code);
  return m_device.createShaderModule(createInfo);
}

vk::DescriptorSetLayout
VulkanPipelineContext::CreateDescriptorSetLayout(const DescriptorSetArgument& layoutBinding) {
  if (m_descriptorSetLayoutCache.find(layoutBinding) != m_descriptorSetLayoutCache.end()) {
    // the descriptorSetLayout has been create
    auto vkLayout = m_descriptorSetLayoutCache.at(layoutBinding);
    m_descriptorSetLayoutCount[vkLayout]++;
    return vkLayout;
  }

  // create the descriptorSetLayout and insert it
  std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
  for (const auto& [bingingPoint, type] : layoutBinding.m_bindingInfo) {
    vk::DescriptorSetLayoutBinding binding;
    binding.setBinding(bingingPoint);

    switch (type) {
      case DescriptorType::UNIFORM_BUFFER:
        binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        break;
      case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
        binding.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
        break;
      case DescriptorType::IMAGE:
        binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        break;
      case DescriptorType::STORAGE_BUFFER:
        binding.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        break;
    }
    binding.setDescriptorCount(1);
    binding.setStageFlags(vk::ShaderStageFlagBits::eAll);
    vkBindings.push_back(binding);
  }

  vk::DescriptorSetLayoutCreateInfo createInfo;
  createInfo.setBindings(vkBindings);
  auto vkLayout = m_device.createDescriptorSetLayout(createInfo);

  m_descriptorSetLayoutCount[vkLayout] = 1;
  m_descriptorSetLayoutCache.insert_or_assign(layoutBinding, vkLayout);
  return vkLayout;
}

void
VulkanPipelineContext::DestroyDescriptorSetLayout(const vk::DescriptorSetLayout& layout) {
  m_descriptorSetLayoutCount[layout]--;
  if (m_descriptorSetLayoutCount.at(layout) == 0) {
    // delete the descriptorSetLayout
    auto iter = std::find_if(m_descriptorSetLayoutCache.begin(), m_descriptorSetLayoutCache.end(), [&](auto& cache) {
      auto vkLayout = cache.second;
      return vkLayout == layout;
    });

    m_descriptorSetLayoutCache.erase(iter);
    m_descriptorSetLayoutCount.erase(layout);
    m_device.destroyDescriptorSetLayout(layout);
  }
}

uintptr_t
VulkanPipelineContext::CreateDescriptorSet(const DescriptorSetArgument& argument) {
  auto vkLayout = CreateDescriptorSetLayout(argument);

  auto vkSet = m_descriptorAllocate.Allocate(vkLayout);
  m_descriptorSets.insert_or_assign(vkSet, vkLayout);
  return reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(vkSet));
}

void
VulkanPipelineContext::DestroyDescriptorSet(uintptr_t set) {
  auto vkSet = static_cast<vk::DescriptorSet>(reinterpret_cast<VkDescriptorSet>(set));

  m_descriptorAllocate.Delete(vkSet);
  DestroyDescriptorSetLayout(m_descriptorSets[vkSet]);
  m_descriptorSets.erase(vkSet);
}

/**
 * pipeline
 */

// graphics pipeline
uintptr_t
VulkanPipelineContext::CreatePipeline(const GraphicsPipeLineCreateInfo& createInfo) {
  // auto* graphicsPipeline = new VulkanPipeline();
  auto vkRenderPass = CreateRenderPass(createInfo.outputRenderTarget, vk::PipelineBindPoint::eGraphics);

  // render pass
  vk::GraphicsPipelineCreateInfo vkCreateInfo;
  vkCreateInfo.setRenderPass(vkRenderPass);

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
  std::vector<vk::ShaderModule> shaderModules;
  for (auto& shaderStageCreateInfo : createInfo.shaderStageCreateInfo) {
    vk::PipelineShaderStageCreateInfo info;

    // convert shader
    auto module = CreateShaderModule(shaderStageCreateInfo.code, shaderStageCreateInfo.stage);
    shaderModules.push_back(module);
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
  std::array<vk::Viewport, 1> viewports = {vk::Viewport(0, 0, 800, 600)};
  std::array<vk::Rect2D, 1> scissors = {vk::Rect2D({0, 0}, {800, 600})};
  vk::PipelineViewportStateCreateInfo vkViewportStateCreateInfo;
  vkViewportStateCreateInfo.setViewports(viewports);
  vkViewportStateCreateInfo.setScissors(scissors);
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
  vkDepthStencilStateCreateInfo.setMaxDepthBounds(1);
  vkDepthStencilStateCreateInfo.setMinDepthBounds(-1);
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
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  for (auto& argument : createInfo.layout) {
    auto vkDescriptorSet = CreateDescriptorSetLayout(argument);
    descriptorSetLayouts.push_back(vkDescriptorSet);
  }
  auto vkPipelineLayout = CreatePipelineLayout(descriptorSetLayouts);
  vkCreateInfo.setLayout(vkPipelineLayout);

  auto graphicsPipeline = m_device.createGraphicsPipeline(nullptr, vkCreateInfo);
  if (graphicsPipeline.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create the vulkan graphics pipepine");
  }

  VulkanGraphicsPipelineInfo graphicsPipelineInfo;
  graphicsPipelineInfo.shaderModule = shaderModules;
  graphicsPipelineInfo.vkPipelineLayout = vkPipelineLayout;
  graphicsPipelineInfo.vkRenderPass = vkRenderPass;
  m_graphicsPipeline[graphicsPipeline.value] = std::move(graphicsPipelineInfo);

  return reinterpret_cast<uintptr_t>(static_cast<VkPipeline>(graphicsPipeline.value));
}

uintptr_t
VulkanPipelineContext::CreatePipeline(const ComputePipelineCreateInfo& createInfo) {
  vk::ComputePipelineCreateInfo vkCreateInfo;

  vk::PipelineShaderStageCreateInfo shaderStageInfo;

  // convert shader
  auto module = CreateShaderModule(createInfo.computeShaderStage.code, createInfo.computeShaderStage.stage);
  shaderStageInfo.setModule(module);
  shaderStageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
  shaderStageInfo.setPName(createInfo.computeShaderStage.interName.c_str());
  vkCreateInfo.setStage(shaderStageInfo);

  // layout
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  for (auto& argument : createInfo.layout) {
    auto vkDescriptorSet = CreateDescriptorSetLayout(argument);
    descriptorSetLayouts.push_back(vkDescriptorSet);
  }
  auto vkPipelineLayout = CreatePipelineLayout(descriptorSetLayouts);
  vkCreateInfo.setLayout(vkPipelineLayout);

  auto vkPipeline = m_device.createComputePipeline(nullptr, vkCreateInfo);

  VulkanComputePipelineInfo computePipelineInfo;
  computePipelineInfo.vkPipelineLayout = vkPipelineLayout;
  computePipelineInfo.shaderModule = {module};

  m_computePipeline[vkPipeline.value] = std::move(computePipelineInfo);

  return reinterpret_cast<uintptr_t>(static_cast<VkPipeline>(vkPipeline.value));
}

void
VulkanPipelineContext::DestroyPipeline(uintptr_t pipeline) {
  auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(pipeline));
  m_device.destroyPipeline(vkPipeline);

  vk::PipelineLayout pipelineLayout;
  std::vector<vk::ShaderModule> modules;
  if (auto iter = m_graphicsPipeline.find(vkPipeline); iter != m_graphicsPipeline.end()) {
    auto& pipelineInfo = iter->second;
    m_device.destroyRenderPass(pipelineInfo.vkRenderPass);
    for (auto& vkModule : pipelineInfo.shaderModule) {
      m_device.destroyShaderModule(vkModule);
    }

    pipelineLayout = pipelineInfo.vkPipelineLayout;

    m_graphicsPipeline.erase(vkPipeline);

  } else if (auto iter = m_computePipeline.find(vkPipeline); iter != m_computePipeline.end()) {
    auto& pipelineInfo = iter->second;
    pipelineLayout = pipelineInfo.vkPipelineLayout;

    for (auto& vkModule : pipelineInfo.shaderModule) {
      m_device.destroyShaderModule(vkModule);
    }

    m_computePipeline.erase(vkPipeline);
  }

  DestroyPipelineLayout(pipelineLayout);
}

FrameBuffer*
VulkanPipelineContext::CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) {
  auto vkPipeline = static_cast<vk::Pipeline>(reinterpret_cast<VkPipeline>(createInfo.pipeline));
  if (m_graphicsPipeline.find(vkPipeline) == m_graphicsPipeline.end()) {
    LOG(ERROR) << "can't find pipeline for create framebuffer";
    return nullptr;
  }
  auto vkRenderPass = m_graphicsPipeline[vkPipeline].vkRenderPass;

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

    description.setFormat(GetVkFormat(colorTargetDesc.format));
    description.setSamples(ConvertToVulkanSampleCount(colorTargetDesc.sampleCount));

    SetAttachmentLayout(description, colorTargetDesc.initAction, colorTargetDesc.finalAction, colorTargetDesc.usage);
    attachmentDescriptions.push_back(description);
  }

  // set depth attachment
  if (renderTargetDescs.depthAttachments.has_value()) {
    const auto& depthAttachment = *renderTargetDescs.depthAttachments;
    vk::AttachmentDescription description;
    depthAttachmentReference = vk::AttachmentReference();
    depthAttachmentReference->setAttachment(attachmentDescriptions.size());
    depthAttachmentReference->setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    description.setFormat(GetVkFormat(ImageFormat::DEPTH));
    description.setSamples(ConvertToVulkanSampleCount(depthAttachment.sampleCount));

    SetAttachmentLayout(description, depthAttachment.initAction, depthAttachment.finalAction, depthAttachment.usage);
    attachmentDescriptions.push_back(description);
  }

  // set resolve attachment
  for (const auto& resolveAttachment : renderTargetDescs.resolveAttachments) {
    vk::AttachmentDescription description;
    vk::AttachmentReference reference;
    reference.setAttachment(attachmentDescriptions.size());
    reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    resolveAttachmentReference.push_back(reference);

    description.setFormat(GetVkFormat(resolveAttachment.format));
    description.setSamples(ConvertToVulkanSampleCount(resolveAttachment.sampleCount));
    SetAttachmentLayout(description, resolveAttachment.initAction, resolveAttachment.finalAction,
                        resolveAttachment.usage);
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
VulkanPipelineContext::CreatePipelineLayout(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts) {
  if (m_pipelineLayoutCache.find(descriptorSetLayouts) != m_pipelineLayoutCache.end()) {
    // pipeline layout has been created
    auto vkLayout = m_pipelineLayoutCache.at(descriptorSetLayouts);
    m_pipelineLayoutCount[vkLayout]++;
    return vkLayout;
  }

  // create the new pipeline layout
  vk::PipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
  vkPipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayouts);

  auto vkLayout = m_device.createPipelineLayout(vkPipelineLayoutCreateInfo);
  m_pipelineLayoutCache.insert_or_assign(descriptorSetLayouts, vkLayout);
  m_pipelineLayoutCount[vkLayout]++;

  return vkLayout;
}

void
VulkanPipelineContext::DestroyPipelineLayout(const vk::PipelineLayout& layout) {
  m_pipelineLayoutCount[layout]--;
  if (m_pipelineLayoutCount.at(layout) == 0) {
    // delete the descriptorSetLayout
    auto iter = std::find_if(m_pipelineLayoutCache.begin(), m_pipelineLayoutCache.end(), [&](auto& cache) {
      auto vkLayout = cache.second;
      return vkLayout == layout;
    });

    for (auto descriptorSetLayout : iter->first) {
      DestroyDescriptorSetLayout(descriptorSetLayout);
    }

    m_pipelineLayoutCache.erase(iter);
    m_pipelineLayoutCount.erase(layout);
    m_device.destroyPipelineLayout(layout);
  }
}

void
VulkanPipelineContext::BindBuffer(const BindBufferInfo& bindBufferInfo) {
  auto descriptorSet = bindBufferInfo.descriptorSet;
  auto vkDescriptorSet = static_cast<vk::DescriptorSet>(reinterpret_cast<VkDescriptorSet>(descriptorSet));
  if (m_descriptorSets.find(vkDescriptorSet) == m_descriptorSets.end()) {
    constexpr static std::string_view errMsg = "can't find descriptorSet";
    LOG(ERROR) << errMsg;
    throw std::runtime_error(errMsg.data());
  }

  auto* vulkanBuffer = static_cast<VulkanBuffer*>(bindBufferInfo.buffer);
  const auto& descriptorType = bindBufferInfo.descriptorType;

  vk::DescriptorBufferInfo vkDescriptorBufferInfo;
  vkDescriptorBufferInfo.setBuffer(vulkanBuffer->vkBuffer);
  vkDescriptorBufferInfo.setOffset(bindBufferInfo.offset);
  vkDescriptorBufferInfo.setRange(vulkanBuffer->size);

  vk::WriteDescriptorSet vkWriteDescriptorSet;
  vkWriteDescriptorSet.setDstBinding(bindBufferInfo.bindingPoint);
  vkWriteDescriptorSet.setDescriptorCount(1);
  vkWriteDescriptorSet.setBufferInfo(vkDescriptorBufferInfo);
  vkWriteDescriptorSet.setDstArrayElement(bindBufferInfo.arrayElement);
  vkWriteDescriptorSet.setDstSet(vkDescriptorSet);

  switch (descriptorType) {
    case DescriptorType::UNIFORM_BUFFER:
      vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eUniformBuffer);
      break;
    case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
      vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
      break;
    case DescriptorType::IMAGE:
      LOG(ERROR) << "can't bind image when binding buffer";
      return;
    case DescriptorType::STORAGE_BUFFER:
      vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eStorageBuffer);
      break;
  }

  m_device.updateDescriptorSets(vkWriteDescriptorSet, nullptr);
}

void
VulkanPipelineContext::BindImage(const BindImageInfo& bindImageInfo) {
  auto descriptorSet = bindImageInfo.descriptorSet;
  auto vkDescriptorSet = static_cast<vk::DescriptorSet>(reinterpret_cast<VkDescriptorSet>(descriptorSet));
  if (m_descriptorSets.find(vkDescriptorSet) == m_descriptorSets.end()) {
    constexpr static std::string_view errMsg = "can't find descriptorSet";
    LOG(ERROR) << errMsg;
    throw std::runtime_error(errMsg.data());
  }

  auto* vulkanImageView = static_cast<VulkanImageView*>(bindImageInfo.imageView);
  const auto& vkImageView = vulkanImageView->vkImageView;
  auto vkSampler = static_cast<vk::Sampler>(reinterpret_cast<VkSampler>(bindImageInfo.sampler));

  vk::DescriptorImageInfo vkDescriptorImageInfo;
  vkDescriptorImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
  vkDescriptorImageInfo.setImageView(vkImageView);
  vkDescriptorImageInfo.setSampler(vkSampler);

  vk::WriteDescriptorSet vkWriteDescriptorSet;
  vkWriteDescriptorSet.setDstSet(vkDescriptorSet);
  vkWriteDescriptorSet.setDstBinding(bindImageInfo.bindingPoint);
  vkWriteDescriptorSet.setDescriptorCount(1);
  vkWriteDescriptorSet.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
  vkWriteDescriptorSet.setImageInfo(vkDescriptorImageInfo);
  vkWriteDescriptorSet.setDstArrayElement(0);

  m_device.updateDescriptorSets(vkWriteDescriptorSet, nullptr);
}
}  // namespace Marbas
