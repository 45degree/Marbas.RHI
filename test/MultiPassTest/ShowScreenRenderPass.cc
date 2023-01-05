#include "ShowScreenRenderPass.hpp"

namespace Marbas {

ShowScreenRenderPass::ShowScreenRenderPass(RHIFactory* rhiFactory, ImageView* imageView) : RenderPassBase(rhiFactory) {
  // m_texture = imageView;
  CreateDescirptorSet(imageView);
  m_pipeline = CreatePipeline();
}

ShowScreenRenderPass::~ShowScreenRenderPass() {
  m_pipelineContext->DestroyDescriptorSetLayout(m_descriptorSetLayout);
  m_pipelineContext->DestroyDescriptorPool(m_descriptorPool);
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

  std::vector<DescriptorSetLayoutBinding> bindings = {DescriptorSetLayoutBinding{
      .bindingPoint = 0,
      .descriptorType = DescriptorType::IMAGE,
  }};
  m_descriptorSetLayout = m_pipelineContext->CreateDescriptorSetLayout(bindings);

  std::vector<DescriptorPoolSize> poolSize{
      DescriptorPoolSize{
          .type = DescriptorType::IMAGE,
          .size = 1,
      },
  };
  m_descriptorPool = m_pipelineContext->CreateDescriptorPool(poolSize);

  m_descritorSet = m_pipelineContext->CreateDescriptorSet(m_descriptorPool, m_descriptorSetLayout);

  m_pipelineContext->BindImage(BindImageInfo{
      .descriptorSet = m_descritorSet,
      .bindingPoint = 0,
      .imageView = imageView,
      .sampler = m_sampler,
  });
}

Pipeline*
ShowScreenRenderPass::CreatePipeline() {
  m_vertexShaderModule = CreateShaderModule("showBoxScreen.vert.glsl.spv");
  m_fragmentShaderModule = CreateShaderModule("showBoxScreen.frag.glsl.spv");

  std::vector<ShaderStageCreateInfo> shaderStageCreateInfos = {
      ShaderStageCreateInfo{
          .stage = ShaderType::VERTEX_SHADER,
          .code = m_vertexShaderModule,
          .interName = "main",
      },
      ShaderStageCreateInfo{
          .stage = ShaderType::FRAGMENT_SHADER,
          .code = m_fragmentShaderModule,
          .interName = "main",
      },
  };

  GraphicsPipeLineCreateInfo createInfo;
  createInfo.vertexInputLayout.viewDesc = {
      InputElementView{
          .binding = 0,
          .stride = 0,
          .inputClass = VertexInputClass::INSTANCE,
      },
  };
  createInfo.shaderStageCreateInfo = shaderStageCreateInfos;
  createInfo.depthStencilInfo.depthTestEnable = false;
  createInfo.layout = m_descriptorSetLayout;
  createInfo.multisampleCreateInfo.rasterizationSamples = SampleCount::BIT1;
  createInfo.outputRenderTarget.colorAttachments = {ColorTargetDesc{
      .isClear = true,
      .isPresent = true,
      .sampleCount = SampleCount::BIT1,
      .format = ImageFormat::BGRA,
  }};

  createInfo.blendInfo.attachments = {BlendAttachment{.blendEnable = false}};

  return m_pipelineContext->CreatePipeline(createInfo);
}

void
ShowScreenRenderPass::RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) {
  commandBuffer->Begin();
  commandBuffer->BeginPipeline(m_pipeline, m_frameBuffers[frameIndex], {{0, 0, 0, 1}});
  commandBuffer->BindDescriptorSet(m_pipeline, m_descritorSet);
  commandBuffer->Draw(6, 1, 0, 0);
  commandBuffer->EndPipeline(m_pipeline);
  commandBuffer->End();
}

}  // namespace Marbas
