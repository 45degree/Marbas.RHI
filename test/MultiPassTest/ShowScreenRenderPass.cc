#include "ShowScreenRenderPass.hpp"

namespace Marbas {

ShowScreenRenderPass::ShowScreenRenderPass(RHIFactory* rhiFactory, ImageView* imageView) : RenderPassBase(rhiFactory) {
  // m_texture = imageView;
  CreateDescirptorSet(imageView);
  m_pipeline = CreatePipeline();
}

ShowScreenRenderPass::~ShowScreenRenderPass() {
  m_pipelineContext->DestroyPipeline(m_pipeline);
  m_pipelineContext->DestroySampler(m_sampler);
}

void
ShowScreenRenderPass::CreateDescirptorSet(ImageView* imageView) {
  m_sampler = m_pipelineContext->CreateSampler(SamplerCreateInfo{
      .filter = Marbas::Filter::MIN_MAG_MIP_LINEAR,
      .addressU = Marbas::SamplerAddressMode::WRAP,
      .addressV = Marbas::SamplerAddressMode::WRAP,
      .addressW = Marbas::SamplerAddressMode::WRAP,
      .comparisonOp = Marbas::ComparisonOp::ALWAYS,
      .mipLodBias = 0,
      .minLod = 0,
      .maxLod = 0,
      .borderColor = Marbas::BorderColor::IntOpaqueBlack,
  });

  m_argument.Bind(0, DescriptorType::IMAGE);

  m_descritorSet = m_pipelineContext->CreateDescriptorSet(m_argument);

  m_pipelineContext->BindImage(BindImageInfo{
      .descriptorSet = m_descritorSet,
      .bindingPoint = 0,
      .imageView = imageView,
      .sampler = m_sampler,
  });
}

uintptr_t
ShowScreenRenderPass::CreatePipeline() {
  std::vector shaderCreateInfos = {
      ShaderStageCreateInfo("showBoxScreen.vert.glsl.spv", ShaderType::VERTEX_SHADER),
      ShaderStageCreateInfo("showBoxScreen.frag.glsl.spv", ShaderType::FRAGMENT_SHADER),
  };

  GraphicsPipeLineCreateInfo createInfo;
  createInfo.vertexInputLayout.viewDesc = {
      InputElementView{
          .binding = 0,
          .stride = 0,
          .inputClass = VertexInputClass::INSTANCE,
      },
  };
  createInfo.shaderStageCreateInfo = shaderCreateInfos;
  createInfo.depthStencilInfo.depthTestEnable = false;
  createInfo.layout = {m_argument};
  createInfo.multisampleCreateInfo.rasterizationSamples = SampleCount::BIT1;
  createInfo.outputRenderTarget.colorAttachments = {ColorTargetDesc{
      .initAction = AttachmentInitAction::CLEAR,
      .finalAction = AttachmentFinalAction::PRESENT,
      .usage = ImageUsageFlags::COLOR_RENDER_TARGET,
      .sampleCount = SampleCount::BIT1,
      .format = ImageFormat::BGRA,
  }};

  createInfo.blendInfo.attachments = {BlendAttachment{.blendEnable = false}};

  return m_pipelineContext->CreatePipeline(createInfo);
}

void
ShowScreenRenderPass::RecordCommand(GraphicsCommandBuffer* commandBuffer, uint32_t frameIndex) {
  auto* frameBuffer = m_frameBuffers[frameIndex];
  const auto& height = frameBuffer->height;
  const auto& width = frameBuffer->width;

  Marbas::ViewportInfo viewportInfo;
  viewportInfo.x = 0;
  viewportInfo.y = 0;
  viewportInfo.width = static_cast<float>(width);
  viewportInfo.height = static_cast<float>(height);
  viewportInfo.minDepth = 0;
  viewportInfo.maxDepth = 1;

  Marbas::ScissorInfo scissorInfo;
  scissorInfo.x = 0;
  scissorInfo.y = 0;
  scissorInfo.width = width;
  scissorInfo.height = height;
  commandBuffer->Begin();
  commandBuffer->BeginPipeline(m_pipeline, m_frameBuffers[frameIndex], {{0, 0, 0, 1}});
  commandBuffer->SetViewports({&viewportInfo, 1});
  commandBuffer->SetScissors({&scissorInfo, 1});
  commandBuffer->BindDescriptorSet(m_pipeline, {m_descritorSet});
  commandBuffer->Draw(6, 1, 0, 0);
  commandBuffer->EndPipeline(m_pipeline);
  commandBuffer->End();
}

}  // namespace Marbas
