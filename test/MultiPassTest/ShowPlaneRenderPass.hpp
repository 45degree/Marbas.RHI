#pragma once

#include "RenderPassBase.hpp"

namespace Marbas {

class ShowPlaneRenderPass final : public RenderPassBase {
 public:
  explicit ShowPlaneRenderPass(RHIFactory* rhiFactory);
  ~ShowPlaneRenderPass() override;

 public:
  void
  RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) override;

  Pipeline*
  CreatePipeline() override;

 private:
  void
  LoadImage();

  void
  CreateDescriptor();

  void
  ReadShader();

 private:
  CameraInfo m_cameraInfo;
  Model m_model;

  Buffer* m_uniformBuffer = nullptr;
  Buffer* m_vertexBuffer = nullptr;
  std::vector<char> m_vertexShaderModule;
  std::vector<char> m_fragmentShaderModule;

  Image* m_texture;
  ImageView* m_textureView;
  Sampler* m_sampler;
  DescriptorSetLayout* m_descriptorSetLayout = nullptr;
  DescriptorPool* m_descritporPool = nullptr;
  DescriptorSet* m_descriptorSet = nullptr;

  std::string m_texturePath = "metal.png";
};

}  // namespace Marbas
