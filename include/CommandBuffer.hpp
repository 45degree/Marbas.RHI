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

#include <span>

#include "DescriptorSet.hpp"
#include "Pipeline.hpp"
#include "Synchronic.hpp"

namespace Marbas {

struct ClearValue {
  explicit ClearValue(const std::array<float, 4>& colorValue) { clearValue = colorValue; }
  explicit ClearValue(const std::array<float, 2>& depthStencilValue) { clearValue = depthStencilValue; }
  ClearValue(float r, float g, float b, float a) { clearValue = std::array<float, 4>{r, g, b, a}; }
  ClearValue(float depth, float stencil) { clearValue = std::array<float, 2>{depth, stencil}; }

  void
  SetClearColor(const std::array<float, 4>& colorValue) {
    clearValue = colorValue;
  }

  void
  SetDepthStencilValue(const std::array<float, 2>& depthStencilValue) {
    clearValue = depthStencilValue;
  }

  std::variant<std::array<float, 4>, std::array<float, 2>> clearValue;
};

class GraphicsCommandBuffer {
 public:
  virtual ~GraphicsCommandBuffer() = default;

 public:
  virtual void
  Begin() = 0;

  virtual void
  End() = 0;

  virtual void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) = 0;

  virtual void
  BeginPipeline(uintptr_t pipeline, FrameBuffer* frameBuffer, const std::vector<ClearValue>& clearColor) = 0;

  virtual void
  EndPipeline(uintptr_t pipeline) = 0;

  virtual void
  BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t>& sets) = 0;

  virtual void
  PushConstant(uintptr_t pipeline, const void* data, uint32_t size, uint32_t offset) = 0;

 public:
  virtual void
  SetViewports(std::span<ViewportInfo> viewportInfos) = 0;

  virtual void
  SetScissors(std::span<ScissorInfo> scissorInfos) = 0;
  virtual void
  BindVertexBuffer(Buffer* buffer) = 0;

  virtual void
  BindIndexBuffer(Buffer* buffer) = 0;

  /**
   * @brief Draw primitives
   *
   * @param vertexCount the number of vertices to draw.
   * @param instanceCount the number of instances to draw.
   * @param firstVertex the index of the first vertex to draw.
   * @param firstInstance the instance ID of the first instance to draw.
   */
  virtual void
  Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

  /**
   * @brief Draw primitives with indexed vertices
   *
   * @param indexCount the number of vertices to draw.
   * @param instanceCount the number of instances to draw.
   * @param firstIndex the base index within the index buffer
   * @param vertexOffset A value added to each index before reading a vertex from the vertex buffer.
   * @param firstInstance the instance ID of the first instance to draw.
   */
  virtual void
  DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
              uint32_t firstInstance) = 0;
};

class ComputeCommandBuffer {
 public:
  virtual ~ComputeCommandBuffer() = default;

 public:
  virtual void
  Begin() = 0;

  virtual void
  End() = 0;

  virtual void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) = 0;

  virtual void
  BeginPipeline(uintptr_t pipeline) = 0;

  virtual void
  EndPipeline(uintptr_t pipeline) = 0;

  virtual void
  BindDescriptorSet(uintptr_t pipeline, const std::vector<uintptr_t>& sets) = 0;

 public:
  virtual void
  Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
};

}  // namespace Marbas
