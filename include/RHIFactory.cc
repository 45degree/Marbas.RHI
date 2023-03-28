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
#include "RHIFactory.hpp"

#include <glog/logging.h>

#ifdef USE_VULKAN
#include "Vulkan/VulkanRHIFactory.hpp"
#endif

#ifdef USE_D3D12
#include "DirectX12/DirectX12RHIFactory.hpp"
#endif

namespace Marbas {

std::unique_ptr<RHIFactory>
RHIFactory::CreateInstance(const RendererType& rendererTyoe) {
#ifdef USE_VULKAN
  if (rendererTyoe == RendererType::VULKAN) {
    return std::unique_ptr<RHIFactory>(new VulkanRHIFactory());
  }
#endif

  DLOG_ASSERT(false);
  return nullptr;
}

}  // namespace Marbas
