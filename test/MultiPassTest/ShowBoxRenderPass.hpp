#include "RenderPassBase.hpp"

namespace Marbas {

class ShowBoxRenderPass final : public RenderPassBase {
 public:
  explicit ShowBoxRenderPass(RHIFactory* rhiFactory);
  ~ShowBoxRenderPass() override {
    m_bufferContext->DestroyBuffer(m_vertexBuffer);
    m_bufferContext->DestroyBuffer(m_uniformBuffer);
    m_bufferContext->DestroyImage(m_texture);
    m_bufferContext->DestroyImageView(m_textureView);
    m_pipelineContext->DestroySampler(m_sampler);
    m_pipelineContext->DestroyPipeline(m_pipeline);
  }

 public:
  void
  RecordCommand(GraphicsCommandBuffer* commandBuffer, uint32_t frameIndex) override;

  uintptr_t
  CreatePipeline() override;

  void
  CreateDescriptorSetLayout();

  void
  CreateDescriptorSet();

 private:
  CameraInfo m_cameraInfo;
  Model m_model;
  std::string m_texturePath = "container.jpg";
  Image* m_texture = nullptr;
  ImageView* m_textureView = nullptr;
  uintptr_t m_sampler;

  std::vector<char> m_vertexShaderModule;
  std::vector<char> m_fragmentShaderModule;

  Buffer* m_vertexBuffer = nullptr;
  Buffer* m_uniformBuffer = nullptr;
  DescriptorSetArgument m_argument;
  uintptr_t m_descriptorSet;
};

}  // namespace Marbas
