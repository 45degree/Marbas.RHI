#include "ShowPlaneRenderPass.hpp"

namespace Marbas {

ShowPlaneRenderPass::ShowPlaneRenderPass(RHIFactory* rhiFactory) : RenderPassBase(rhiFactory) {
  LoadImage();
  CreateDescriptor();
  m_model.m_vertices = {
      // positions          // texture Coords
      Vertex{{5.0f, -0.5f, 5.0f}, {2.0f, 0.0f}},    //
      Vertex{{-5.0f, -0.5f, 5.0f}, {0.0f, 0.0f}},   //
      Vertex{{-5.0f, -0.5f, -5.0f}, {0.0f, 2.0f}},  //
      Vertex{{5.0f, -0.5f, 5.0f}, {2.0f, 0.0f}},    //
      Vertex{{-5.0f, -0.5f, -5.0f}, {0.0f, 2.0f}},  //
      Vertex{{5.0f, -0.5f, -5.0f}, {2.0f, 2.0f}},   //
  };
  m_cameraInfo.project[1][1] *= -1;

  auto vaoSize = sizeof(Vertex) * m_model.m_vertices.size();
  m_vertexBuffer = m_bufferContext->CreateBuffer(BufferType::VERTEX_BUFFER, m_model.m_vertices.data(), vaoSize, true);
  m_uniformBuffer = m_bufferContext->CreateBuffer(BufferType::UNIFORM_BUFFER, &m_cameraInfo, sizeof(CameraInfo), true);

  m_pipelineContext->BindBuffer(BindBufferInfo{
      .descriptorSet = m_descriptorSet,
      .descriptorType = DescriptorType::UNIFORM_BUFFER,
      .bindingPoint = 0,
      .buffer = m_uniformBuffer,
      .offset = 0,
      .arrayElement = 0,
  });
  m_pipelineContext->BindImage(BindImageInfo{
      .descriptorSet = m_descriptorSet,
      .bindingPoint = 1,
      .imageView = m_textureView,
      .sampler = m_sampler,
  });

  ReadShader();
  m_pipeline = CreatePipeline();
}

ShowPlaneRenderPass::~ShowPlaneRenderPass() {
  m_bufferContext->DestroyImage(m_texture);
  m_bufferContext->DestroyImageView(m_textureView);
  m_bufferContext->DestroyBuffer(m_uniformBuffer);
  m_bufferContext->DestroyBuffer(m_vertexBuffer);

  m_pipelineContext->DestroyShaderModule(m_vertexShaderModule);
  m_pipelineContext->DestroyShaderModule(m_fragmentShaderModule);
  m_pipelineContext->DestroyDescriptorSetLayout(m_descriptorSetLayout);
  m_pipelineContext->DestroyDescriptorPool(m_descritporPool);
  m_pipelineContext->DestroyPipeline(m_pipeline);
  m_pipelineContext->DestroySampler(m_sampler);
}

Pipeline*
ShowPlaneRenderPass::CreatePipeline() {
  std::vector shaderCreateInfos = {
      ShaderStageCreateInfo{
          .stage = ShaderType::VERTEX_SHADER,
          .shaderModule = m_vertexShaderModule,
          .interName = "main",
      },
      ShaderStageCreateInfo{
          .stage = ShaderType::FRAGMENT_SHADER,
          .shaderModule = m_fragmentShaderModule,
          .interName = "main",
      },
  };

  GraphicsPipeLineCreateInfo createInfo;
  createInfo.multisampleCreateInfo.rasterizationSamples = SampleCount::BIT1;
  createInfo.shaderStageCreateInfo = shaderCreateInfos;
  createInfo.vertexInputLayout.elementDesc = Vertex::GetInputElementDesc();
  createInfo.vertexInputLayout.viewDesc = {InputElementView{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputClass = VertexInputClass::VERTEX,
  }};
  createInfo.blendInfo.attachments = {
      BlendAttachment{.blendEnable = false},
  };
  createInfo.outputRenderTarget.colorAttachments = {Marbas::ColorTargetDesc{
      .isClear = false,
      .isPresent = true,  // TODO: don't present
      .sampleCount = Marbas::SampleCount::BIT1,
      .format = Marbas::ImageFormat::BGRA,
  }};
  createInfo.outputRenderTarget.depthAttachments = Marbas::DepthTargetDesc{
      .isClear = false,
      .sampleCount = Marbas::SampleCount::BIT1,
  };
  createInfo.descriptorSetLayout = m_descriptorSetLayout;
  createInfo.depthStencilInfo.depthTestEnable = false;

  return m_pipelineContext->CreatePipeline(createInfo);
}

void
ShowPlaneRenderPass::RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) {
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
  commandBuffer->BeginPipeline(m_pipeline, m_frameBuffers[frameIndex], {});
  commandBuffer->SetViewports({&viewportInfo, 1});
  commandBuffer->SetScissors({&scissorInfo, 1});
  commandBuffer->BindVertexBuffer(m_vertexBuffer);
  commandBuffer->BindDescriptorSet(m_pipeline, 0, {&m_descriptorSet, 1});
  commandBuffer->Draw(m_model.m_vertices.size(), 1, 0, 0);
  commandBuffer->EndPipeline(m_pipeline);
  commandBuffer->End();
}

void
ShowPlaneRenderPass::CreateDescriptor() {
  std::array bindings = {
      DescriptorSetLayoutBinding{
          .bindingPoint = 0,
          .descriptorType = DescriptorType::UNIFORM_BUFFER,
          .count = 1,
          .visible = DescriptorVisible::ALL,
      },
      DescriptorSetLayoutBinding{
          .bindingPoint = 1,
          .descriptorType = DescriptorType::IMAGE,
          .count = 1,
          .visible = DescriptorVisible::ALL,
      },
  };
  m_descriptorSetLayout = m_pipelineContext->CreateDescriptorSetLayout(bindings);

  std::array descriptorSize = {
      DescriptorPoolSize{
          .type = DescriptorType::UNIFORM_BUFFER,
          .size = 1,
      },
      DescriptorPoolSize{
          .type = DescriptorType::IMAGE,
          .size = 1,
      },
  };
  m_descritporPool = m_pipelineContext->CreateDescriptorPool(descriptorSize, 1);

  m_descriptorSet = m_pipelineContext->CreateDescriptorSet(m_descritporPool, m_descriptorSetLayout);
}

void
ShowPlaneRenderPass::ReadShader() {
  m_vertexShaderModule = m_pipelineContext->CreateShaderModule("showBox.vert.glsl.spv");
  m_fragmentShaderModule = m_pipelineContext->CreateShaderModule("showBox.frag.glsl.spv");
}

void
ShowPlaneRenderPass::LoadImage() {
  auto [texture, lod] = RenderPassBase::LoadImage(m_bufferContext, m_texturePath);
  m_texture = texture;
  m_bufferContext->GenerateMipmap(m_texture, lod);

  m_textureView = m_bufferContext->CreateImageView(ImageViewCreateInfo{
      .image = m_texture,
      .baseLevel = 0,
      .levelCount = lod,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });

  m_sampler = m_pipelineContext->CreateSampler(SamplerCreateInfo{
      .filter = Marbas::Filter::MIN_MAG_MIP_LINEAR,
      .addressU = Marbas::SamplerAddressMode::WRAP,
      .addressV = Marbas::SamplerAddressMode::WRAP,
      .addressW = Marbas::SamplerAddressMode::WRAP,
      .comparisonOp = Marbas::ComparisonOp::ALWAYS,
      .mipLodBias = 0,
      .minLod = 0,
      .maxLod = static_cast<float>(lod),
      .borderColor = Marbas::BorderColor::IntOpaqueBlack,
  });
}

}  // namespace Marbas
