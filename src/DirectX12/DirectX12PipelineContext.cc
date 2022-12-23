/**
 * Copyright 2022.12.21 45degree
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

#include "DirectX12PipelineContext.hpp"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_hlsl.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <unordered_map>

#include "DirectX12Descriptor.hpp"
#include "DirectX12Image.hpp"
#include "DirectX12Pipeline.hpp"
#include "DirectX12Util.hpp"

namespace Marbas {

std::string
DirectX12PipelineContext::ConvertShader(const std::vector<char>& originCode) {
  std::vector<uint32_t> code(std::ceil(originCode.size() / sizeof(uint32_t)));
  memcpy(code.data(), originCode.data(), originCode.size());
  spirv_cross::CompilerHLSL hlsl(code);
  auto resources = hlsl.get_shader_resources();

  for (auto& resource : resources.uniform_buffers) {
    unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
    unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
    printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

    // Modify the decoration to prepare it for GLSL.
    // hlsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

    // Some arbitrary remapping if we want.
    // hlsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
  }

  spirv_cross::CompilerHLSL::Options option;
  option.shader_model = 51;
  hlsl.set_hlsl_options(option);
  return hlsl.compile();
}

DescriptorPool*
DirectX12PipelineContext::CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) {
  D3D12_DESCRIPTOR_HEAP_DESC desc;

  // m_device->CreateDescriptorHeap()
  return nullptr;
}

void
DirectX12PipelineContext::DestroyDescriptorPool(DescriptorPool* descriptorPool) {}

DescriptorSetLayout*
DirectX12PipelineContext::CreateDescriptorSetLayout(std::span<DescriptorSetLayoutBinding> layoutBinding) {
  auto* directX12DescriptorSetLayout = new DirectX12DescriptorSetLayout();
  directX12DescriptorSetLayout->bindings = std::vector(layoutBinding.begin(), layoutBinding.end());
  return directX12DescriptorSetLayout;
}

void
DirectX12PipelineContext::DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout) {
  auto* dxDescriptorSetLayout = static_cast<DirectX12DescriptorSetLayout*>(descriptorSetLayout);
  delete dxDescriptorSetLayout;
}

FrameBuffer*
DirectX12PipelineContext::CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) {
  auto* directX12FrameBuffer = new DirectX12FrameBuffer();

  /**
   * create rtv heap and rtv
   */
  if (!createInfo.attachments.colorAttachments.empty()) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = createInfo.attachments.colorAttachments.size();
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&directX12FrameBuffer->rtvHeap)));

    auto rtvHandle = directX12FrameBuffer->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    auto rtvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (const auto& attachment : createInfo.attachments.colorAttachments) {
      const auto* directX12Image = static_cast<DirectX12Image*>(attachment.image);

      auto* resource = directX12Image->resource;
      D3D12_RENDER_TARGET_VIEW_DESC desc;
      desc.Format = ConvertToDirectX12Format(directX12Image->format);

      // clang-format off
      std::visit( [&](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Attachment2D>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
          desc.Texture2D.MipSlice = value.mipmapLevel;
          desc.Texture2D.PlaneSlice = 0;
        } else if constexpr (std::is_same_v<T, Attachment2DMsaa>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else if constexpr (std::is_same_v<T, Attachment2DArray>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          desc.Texture2DArray.MipSlice = value.mipmapLevel;
          desc.Texture2DArray.FirstArraySlice = value.baseLayer;
          desc.Texture2DArray.ArraySize = value.layerCount;
        } else if constexpr (std::is_same_v<T, Attachment2DMsaaArray>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          desc.Texture2DMSArray.FirstArraySlice = value.baseLayer;
          desc.Texture2DMSArray.ArraySize = value.layerCount;
        } else if constexpr (std::is_same_v<T, Attachment3D>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE3D;
          desc.Texture3D.MipSlice = value.mipmapLevel;
          desc.Texture3D.FirstWSlice = value.baseDepth;
          desc.Texture3D.WSize = value.depthCount;
        }
      }, attachment.subResInfo);
      // clang-format on

      m_device->CreateRenderTargetView(resource, &desc, rtvHandle);
      rtvHandle.ptr += rtvSize;
    }
  }

  /**
   * create dsv heap and dsv
   */
  if (createInfo.attachments.depthStencilAttachment) {
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    const auto& depthAttachment = createInfo.attachments.depthStencilAttachment;
    auto dsvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NodeMask = 0;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&directX12FrameBuffer->dsvHeap)));

    const auto* directX12Image = static_cast<DirectX12Image*>(depthAttachment->image);
    auto* resource = directX12Image->resource;
    D3D12_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = ConvertToDirectX12Format(directX12Image->format);

    // clang-format off
    std::visit([&](auto& value) {
      using T = std::decay_t<decltype(value)>;

      if constexpr (std::is_same_v<T, Attachment2D>) {
        desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = value.mipmapLevel;
      } else if constexpr (std::is_same_v<T, Attachment2DMsaa>) {
        desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DMS;
      } else if constexpr (std::is_same_v<T, Attachment2DArray>) {
        desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = value.mipmapLevel;
        desc.Texture2DArray.FirstArraySlice = value.baseLayer;
        desc.Texture2DArray.ArraySize = value.layerCount;
      } else if constexpr (std::is_same_v<T, Attachment2DMsaaArray>) {
        desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
        desc.Texture2DMSArray.FirstArraySlice = value.baseLayer;
        desc.Texture2DMSArray.ArraySize = value.layerCount;
      }
    }, depthAttachment->subResInfo);
    // clang-format on

    auto dsvHandle = directX12FrameBuffer->dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_device->CreateDepthStencilView(resource, &desc, dsvHandle);
  }

  /**
   * resolve view heap and resolve view
   */
  if (!createInfo.attachments.resolveAttachments.empty()) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = createInfo.attachments.colorAttachments.size();
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&directX12FrameBuffer->resolveHeap)));

    auto rtvHandle = directX12FrameBuffer->resolveHeap->GetCPUDescriptorHandleForHeapStart();
    auto rtvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (const auto& attachment : createInfo.attachments.colorAttachments) {
      const auto* directX12Image = static_cast<DirectX12Image*>(attachment.image);

      auto* resource = directX12Image->resource;
      D3D12_RENDER_TARGET_VIEW_DESC desc;
      desc.Format = ConvertToDirectX12Format(directX12Image->format);

      // clang-format off
      std::visit( [&](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Attachment2D>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
          desc.Texture2D.MipSlice = value.mipmapLevel;
          desc.Texture2D.PlaneSlice = 0;
        } else if constexpr (std::is_same_v<T, Attachment2DMsaa>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else if constexpr (std::is_same_v<T, Attachment2DArray>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          desc.Texture2DArray.MipSlice = value.mipmapLevel;
          desc.Texture2DArray.FirstArraySlice = value.baseLayer;
          desc.Texture2DArray.ArraySize = value.layerCount;
        } else if constexpr (std::is_same_v<T, Attachment2DMsaaArray>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          desc.Texture2DMSArray.FirstArraySlice = value.baseLayer;
          desc.Texture2DMSArray.ArraySize = value.layerCount;
        } else if constexpr (std::is_same_v<T, Attachment3D>) {
          desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE3D;
          desc.Texture3D.MipSlice = value.mipmapLevel;
          desc.Texture3D.FirstWSlice = value.baseDepth;
          desc.Texture3D.WSize = value.depthCount;
        }
      }, attachment.subResInfo);
      // clang-format on

      m_device->CreateRenderTargetView(resource, &desc, rtvHandle);
      rtvHandle.ptr += rtvSize;
    }
  }

  directX12FrameBuffer->height = createInfo.height;
  directX12FrameBuffer->width = createInfo.width;

  // TODO: save pipeline

  return directX12FrameBuffer;
}

void
DirectX12PipelineContext::DestroyFrameBuffer(FrameBuffer* frameBuffer) {
  auto* directX12FrameBuffer = static_cast<DirectX12FrameBuffer*>(frameBuffer);
  directX12FrameBuffer->dsvHeap->Release();
  directX12FrameBuffer->rtvHeap->Release();
  directX12FrameBuffer->resolveHeap->Release();
}

Pipeline*
DirectX12PipelineContext::CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) {
  auto* dxPipeline = new DirectX12Pipeline();

  // create root signature
  D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  std::vector<D3D12_ROOT_PARAMETER1> rootParams;
  for (int space = 0; auto* descriptorSetLayout : createInfo.descriptorSetLayout) {
    auto bindings = static_cast<DirectX12DescriptorSetLayout*>(descriptorSetLayout)->bindings;
    for (const auto& binding : bindings) {
      D3D12_ROOT_PARAMETER1 param;
      param.Descriptor.ShaderRegister = binding.bindingPoint;
      param.Descriptor.RegisterSpace = space;
      param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      switch (binding.descriptorType) {
        case DescriptorType::UNIFORM_BUFFER:
        case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
          param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_UAV;
          break;
        case DescriptorType::IMAGE:
          param.ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
          break;
      }
      rootParams.push_back(param);
    }
    space++;
  }
  rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
  rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  rootSignatureDesc.Desc_1_1.NumParameters = rootParams.size();
  rootSignatureDesc.Desc_1_1.pParameters = rootParams.data();
  rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
  rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;

  ID3DBlob* signature;
  ID3DBlob* error;
  try {
    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
    ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                IID_PPV_ARGS(&dxPipeline->rootSignature)));
    dxPipeline->rootSignature->SetName(L"Hello Triangle Root Signature");
  } catch (std::exception e) {
    auto errStr = static_cast<const char*>(error->GetBufferPointer());
    std::cout << errStr;
    error->Release();
    error = nullptr;
  }
  if (signature) {
    signature->Release();
    signature = nullptr;
  }
  if (error) {
    error->Release();
    error = nullptr;
  }

  // create pso
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  return dxPipeline;
}

void
DirectX12PipelineContext::DestroyPipeline(Pipeline* pipeline) {}

}  // namespace Marbas
