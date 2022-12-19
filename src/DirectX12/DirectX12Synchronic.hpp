/**
<<<<<<< Updated upstream
<<<<<<< Updated upstream
 * Copyright 2022.12.18 45degree
=======
 * Copyright 2022.11.2 45degree
>>>>>>> Stashed changes
=======
 * Copyright 2022.11.2 45degree
>>>>>>> Stashed changes
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
#include "Synchronic.hpp"

namespace Marbas {

struct DirectX12Semaphore final : public Semaphore {
  ID3D12Fence* dxFence;
};

struct DirectX12Fence final : public Fence {
  ID3D12Fence* dxFence;
};

}  // namespace Marbas
