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

enum class CommandBufferUsage {
  GRAPHICS,
  COMPUTE,
  TRANSFER,
};

struct CommandPool {
  CommandBufferUsage usage = CommandBufferUsage::GRAPHICS;
};

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

class CommandBuffer {
 public:
  virtual void
  BindDescriptorSet(const Pipeline* pipeline, DescriptorSet* descriptors) = 0;

  virtual void
  BindVertexBuffer(Buffer* buffer) = 0;

  virtual void
  BindIndexBuffer(Buffer* buffer) = 0;

 public:
  virtual void
  Begin() = 0;

  virtual void
  End() = 0;

  virtual void
  BeginPipeline(Pipeline* pipeline, FrameBuffer* frameBuffer, const std::vector<ClearValue>& clearColor) = 0;

  virtual void
  SetViewports(std::span<ViewportInfo> viewportInfos) = 0;

  virtual void
  SetScissors(std::span<ScissorInfo> scissorInfos) = 0;

  virtual void
  EndPipeline(Pipeline* pipeline) = 0;

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

 public:
  virtual void
  InsertBufferBarrier(const std::vector<BufferBarrier>& barrier) = 0;

  virtual void
  InsertImageBarrier(const std::vector<ImageBarrier>& barriers) = 0;

  virtual void
  TransformImageState(Image* image, ImageState srcState, ImageState dstState) = 0;

 public:
  virtual void
  Submit(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence) = 0;
};

}  // namespace Marbas
