#include "DirectX12PipelineContext.hpp"

namespace Marbas {

DescriptorPool*
DirectX12PipelineContext::CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) {
  D3D12_DESCRIPTOR_HEAP_DESC desc;

  // m_device->CreateDescriptorHeap()
}

void
DirectX12PipelineContext::DestroyDescriptorPool(DescriptorPool* descriptorPool) {}

}  // namespace Marbas
