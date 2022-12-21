/**
 * Copyright 2022.12.21 45degree
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

#include "DirectX12Common.hpp"
#include "Pipeline.hpp"

namespace Marbas {

struct DirectX12FrameBuffer final : public FrameBuffer {
  ID3D12DescriptorHeap* rtvHeap;
  ID3D12DescriptorHeap* dsvHeap;
  ID3D12DescriptorHeap* resolveHeap;
};

struct DirectX12Pipeline final : public Pipeline {
  ID3D12PipelineState* pipelineState;
};

}  // namespace Marbas
