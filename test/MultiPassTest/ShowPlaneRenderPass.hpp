#pragma once

#include "RenderPassBase.hpp"

namespace Marbas {

class ShowPlaneRenderPass final : public RenderPassBase {
 public:
  explicit ShowPlaneRenderPass(RHIFactory* rhiFactory);
  ~ShowPlaneRenderPass() override;

 public:
  void
  RecordCommand(GraphicsCommandBuffer* commandBuffer, uint32_t frameIndex) override;

  uintptr_t
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
  DescriptorSetArgument m_argument;
  uintptr_t m_sampler;
  uintptr_t m_descriptorSet;

  std::string m_texturePath = "metal.png";
};

}  // namespace Marbas
