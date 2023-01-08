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
#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <span>

#include "Synchronic.hpp"

namespace Marbas {

struct ImguiRenderDataInfo {
  const Semaphore* waitSemaphore = nullptr;
  const Semaphore* signalSemaphore = nullptr;
};

class ImguiContext {
 public:
  ImguiContext() = default;
  virtual ~ImguiContext() = default;

 public:
  virtual ImTextureID
  CreateImGuiImage(ImageView* imageView) = 0;

  virtual void
  DestroyImGuiImage(ImTextureID imTextureId) = 0;

  virtual void
  SetRenderResultImage(const std::vector<ImageView*>& resultImageViews) {
    m_renderResultImageViews = resultImageViews;
  }

  virtual void
  Resize(uint32_t width, uint32_t height) = 0;

  virtual void
  ClearUp() = 0;

  virtual void
  SetUpImguiBackend(GLFWwindow* windows) = 0;

  virtual void
  NewFrame() = 0;

  virtual void
  RenderData(uint32_t imageIndex, const ImguiRenderDataInfo& renderInfo) = 0;

 protected:
  std::vector<ImageView*> m_renderResultImageViews;
};

}  // namespace Marbas
