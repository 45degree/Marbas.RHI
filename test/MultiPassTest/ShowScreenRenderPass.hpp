#pragma once

#include "RenderPassBase.hpp"

namespace Marbas {

class ShowScreenRenderPass final : public RenderPassBase {
 public:
  explicit ShowScreenRenderPass(RHIFactory* rhiFactory, ImageView* imageView);
  ~ShowScreenRenderPass() override;

 public:
  void
  RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) override;

  Pipeline*
  CreatePipeline() override;

  void
  CreateDescirptorSet(ImageView* imageView);

 private:
  std::vector<char> m_vertexShaderModule;
  std::vector<char> m_fragmentShaderModule;
  Sampler* m_sampler = nullptr;
  DescriptorPool* m_descriptorPool = nullptr;
  DescriptorSet* m_descritorSet = nullptr;
  DescriptorSetLayout* m_descriptorSetLayout = nullptr;
};

}  // namespace Marbas
