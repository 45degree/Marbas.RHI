/**
 * Copyright 2022.11.2 45degree
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

#include <vector>

#include "Image.hpp"

namespace Marbas {

#ifdef USE_VULKAN
struct SwapchainVulkanData;
#endif

class Swapchain {
  std::vector<Image> images;
  std::vector<ImageView> imageViews;

#ifdef USE_VULKAN
  SwapchainVulkanData* vulkanData;
#endif
};

}  // namespace Marbas
