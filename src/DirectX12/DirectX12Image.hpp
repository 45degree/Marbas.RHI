#pragma once

#include <d3d12.h>
#include <dxgi1_2.h>

#include "Image.hpp"

namespace Marbas {

struct DirectX12Image final : public Image {
  ID3D12Resource* resource = nullptr;
};

struct DirectX12ImageView final : public ImageView {};

}  // namespace Marbas
