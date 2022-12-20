#include "DirectX12PipelineContext.hpp"

#include "DirectX12Image.hpp"
#include "DirectX12Pipeline.hpp"
#include "DirectX12Util.hpp"

namespace Marbas {

DescriptorPool*
DirectX12PipelineContext::CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) {
  D3D12_DESCRIPTOR_HEAP_DESC desc;

  // m_device->CreateDescriptorHeap()
  return nullptr;
}

void
DirectX12PipelineContext::DestroyDescriptorPool(DescriptorPool* descriptorPool) {}

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

}  // namespace Marbas
