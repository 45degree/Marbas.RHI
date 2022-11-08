/**
 * Copyright 2022.11.3 45degree
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

#include "DirectX12RHIFactory.hpp"

#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

#ifndef NDEBUG
#include <dxgidebug.h>
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

namespace Marbas {

void
DirectX12RHIFactory::Init(GLFWwindow* window, uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto a = glfwGetWin32Window(window);

  CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
}

}  // namespace Marbas
