#pragma once

#include "Model.hpp"
#include "RHIFactory.hpp"

namespace Marbas {

struct CameraInfo {
  glm::mat4 model = glm::mat4(1.0);
  glm::mat4 view = glm::lookAt(glm::vec3(5, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 project = glm::perspective(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.);
};

class RenderPassBase {
 public:
  explicit RenderPassBase(RHIFactory* rhifactory);
  virtual ~RenderPassBase() {
    for (const auto& frameBuffer : m_frameBuffers) {
      m_pipelineContext->DestroyFrameBuffer(frameBuffer);
    }
  }

 public:
  virtual void
  RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) = 0;

  virtual Pipeline*
  CreatePipeline() = 0;

  void
  Render(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence, uint32_t frameIndex);

  void
  CreateFrameBuffer(std::vector<FrameBufferCreateInfo>& frameBufferCreateInfo);

 public:
  static std::tuple<Marbas::Image*, uint32_t>
  LoadImage(Marbas::BufferContext* bufferContext, const std::string& imagePath);

 protected:
  Pipeline* m_pipeline = nullptr;
  CommandPool* m_commandPool = nullptr;
  CommandBuffer* m_commandBuffer = nullptr;
  std::vector<FrameBuffer*> m_frameBuffers;
  BufferContext* m_bufferContext = nullptr;
  PipelineContext* m_pipelineContext = nullptr;
};

}  // namespace Marbas
