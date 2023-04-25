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

#pragma once

#include <cstdint>
#include <map>

namespace Marbas {

enum class DescriptorType {
  UNIFORM_BUFFER,
  DYNAMIC_UNIFORM_BUFFER,
  IMAGE,
  STORAGE_BUFFER,
};

class DescriptorSetArgument final {
 public:
  explicit DescriptorSetArgument(const std::map<uint16_t, DescriptorType>& map = {}, uint32_t size = 0)
      : m_bindingInfo(map) {}
  DescriptorSetArgument(const DescriptorSetArgument& obj) = default;
  DescriptorSetArgument(DescriptorSetArgument&& obj) noexcept : m_bindingInfo(std::move(obj.m_bindingInfo)) {}

  DescriptorSetArgument&
  operator=(const DescriptorSetArgument& obj) = default;

  DescriptorSetArgument&
  operator=(DescriptorSetArgument&& obj) noexcept {
    m_bindingInfo = std::move(obj.m_bindingInfo);
    return *this;
  }

 public:
  void
  Bind(uint16_t bindingPoint, DescriptorType type) {
    m_bindingInfo.insert_or_assign(bindingPoint, type);
  }

  bool
  operator==(const DescriptorSetArgument& obj) const {
    return obj.m_bindingInfo == m_bindingInfo;
  }

  void
  Clear() {
    m_bindingInfo.clear();
  }

 public:
  std::map<uint16_t, DescriptorType> m_bindingInfo;
};

}  // namespace Marbas
