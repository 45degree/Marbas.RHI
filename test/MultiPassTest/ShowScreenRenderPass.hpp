#pragma once

#include "RenderPassBase.hpp"

namespace Marbas {

class ShowScreenRenderPass final : public RenderPassBase {
 public:
  explicit ShowScreenRenderPass(RHIFactory* rhiFactory, ImageView* imageView);
  ~ShowScreenRenderPass() override;

 public:
  void
  RecordCommand(GraphicsCommandBuffer* commandBuffer, uint32_t frameIndex) override;

  uintptr_t
  CreatePipeline() override;

  void
  CreateDescirptorSet(ImageView* imageView);

 private:
  std::vector<char> m_vertexShaderModule;
  std::vector<char> m_fragmentShaderModule;
  uintptr_t m_sampler;
  uintptr_t m_descritorSet;
  DescriptorSetArgument m_argument;
};

}  // namespace Marbas
