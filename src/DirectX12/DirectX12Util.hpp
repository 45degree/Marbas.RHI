#pragma once

#include "DirectX12Common.hpp"
#include "Image.hpp"
#include "common.hpp"

namespace Marbas {

FORCE_INLINE DXGI_FORMAT
ConvertToDirectX12Format(ImageFormat format) {
  switch (format) {
    case ImageFormat::RED:
      return DXGI_FORMAT_R8_UNORM;
    case ImageFormat::RG:
      return DXGI_FORMAT_R8G8_UNORM;
    case ImageFormat::RGBA:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::BGRA:
      return DXGI_FORMAT_B8G8R8A8_UNORM;
    case ImageFormat::R32F:
      return DXGI_FORMAT_R32_FLOAT;
    case ImageFormat::RG16F:
      return DXGI_FORMAT_R16G16_FLOAT;
    case ImageFormat::RG32F:
      return DXGI_FORMAT_R32G32_FLOAT;
    case ImageFormat::RGB32F:
      return DXGI_FORMAT_R32G32B32_FLOAT;
    case ImageFormat::DEPTH:
      return DXGI_FORMAT_D32_FLOAT;
    case ImageFormat::RGBA_SRGB:
      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      break;
  }
}

}  // namespace Marbas
