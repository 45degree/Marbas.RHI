/**
 * Copyright 2022.11.5 45degree
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

#include <vulkan/vulkan.hpp>

namespace Marbas {

struct SwapchainVulkanData {
  vk::SwapchainKHR m_swapChain;
  vk::PresentModeKHR m_presentMode;
  uint32_t m_imageCount;

  // swapChain image
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_imageViews;
};

}  // namespace Marbas
