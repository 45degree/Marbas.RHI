/**
 * Copyright 2022.10.31 45degree
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

#include "vulkan/VulkanBufferContext.hpp"

#include "vulkan/vulkanBuffer.hpp"

namespace Marbas {

void
VulkanBufferContext::CreateBuffer(Buffer* buffer, void* data, uint32_t size, bool isStatic) {}

void
VulkanBufferContext::UpdateBuffer(Buffer* buffer, void* data, uint32_t size, uintptr_t offset) {}

void
VulkanBufferContext::DestroyBuffer(Buffer* buffer) {}

}  // namespace Marbas
