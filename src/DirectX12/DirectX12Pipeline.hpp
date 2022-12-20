#pragma once

#include "DirectX12Common.hpp"
#include "Pipeline.hpp"

namespace Marbas {

struct DirectX12FrameBuffer final : public FrameBuffer {
  ID3D12DescriptorHeap* rtvHeap;
  ID3D12DescriptorHeap* dsvHeap;
  ID3D12DescriptorHeap* resolveHeap;
};

}  // namespace Marbas
