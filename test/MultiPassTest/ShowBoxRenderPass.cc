#include "ShowBoxRenderPass.hpp"

#include <stdexcept>

namespace Marbas {

ShowBoxRenderPass::ShowBoxRenderPass(RHIFactory* rhiFactory) : RenderPassBase(rhiFactory) {
  m_model.m_vertices = {
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  //
      Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},   //
      Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
      Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
      Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   //
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  //
                                                    //
      Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
      Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},     //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},     //
      Vertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},    //
      Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
                                                    //
      Vertex{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    //
      Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},   //
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
      Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
      Vertex{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    //
                                                    //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
      Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
      Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},   //
      Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},   //
      Vertex{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},    //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
                                                    //
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
      Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},   //
      Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
      Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
      Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
      Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
                                                    //
      Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   //
      Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
      Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
      Vertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},    //
      Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}    //
  };

  m_cameraInfo.project[1][1] *= -1;
  auto vaoSize = sizeof(Vertex) * m_model.m_vertices.size();
  m_vertexBuffer = m_bufferContext->CreateBuffer(BufferType::VERTEX_BUFFER, m_model.m_vertices.data(), vaoSize, true);
  m_uniformBuffer = m_bufferContext->CreateBuffer(BufferType::UNIFORM_BUFFER, &m_cameraInfo, sizeof(CameraInfo), true);

  auto [texture, lod] = LoadImage(m_bufferContext, m_texturePath);
  m_texture = texture;
  m_bufferContext->GenerateMipmap(m_texture, lod);
  m_textureView = m_bufferContext->CreateImageView(ImageViewCreateInfo{
      .image = m_texture,
      .type = ImageViewType::TEXTURE2D,
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

  m_pipeline = CreatePipeline();
  CreateDescriptorSet();
}

Pipeline*
ShowBoxRenderPass::CreatePipeline() {
  m_descriptorSetLayout = CreateDescriptorSetLayout();

  // create shader
  m_vertexShaderModule = CreateShaderModule("showBox.vert.glsl.spv");
  m_fragmentShaderModule = CreateShaderModule("showBox.frag.glsl.spv");
  Marbas::ShaderStageCreateInfo showBoxVert, showBoxFrag;
  showBoxVert.code = m_vertexShaderModule;
  showBoxVert.stage = Marbas::ShaderType::VERTEX_SHADER;
  showBoxVert.interName = "main";
  showBoxFrag.code = m_fragmentShaderModule;
  showBoxFrag.stage = Marbas::ShaderType::FRAGMENT_SHADER;
  showBoxFrag.interName = "main";

  Marbas::InputElementView elementView;
  elementView.binding = 0;
  elementView.inputClass = Marbas::VertexInputClass::VERTEX;
  elementView.stride = sizeof(Vertex);

  // create render pass
  GraphicsPipeLineCreateInfo createInfo;
  createInfo.vertexInputLayout.elementDesc = Vertex::GetInputElementDesc();
  createInfo.vertexInputLayout.viewDesc = {elementView};
  createInfo.multisampleCreateInfo.rasterizationSamples = Marbas::SampleCount::BIT1;
  createInfo.shaderStageCreateInfo = {showBoxVert, showBoxFrag};
  createInfo.blendInfo.attachments = {
      BlendAttachment{.blendEnable = false},
  };
  createInfo.outputRenderTarget.colorAttachments = {Marbas::ColorTargetDesc{
      .isClear = true,
      .isPresent = false,
      .sampleCount = Marbas::SampleCount::BIT1,
      .format = Marbas::ImageFormat::BGRA,
  }};
  createInfo.outputRenderTarget.depthAttachments = Marbas::DepthTargetDesc{
      .isClear = true,
      .sampleCount = Marbas::SampleCount::BIT1,
  };
  createInfo.layout = m_descriptorSetLayout;
  // createInfo.rasterizationInfo.depthCilpEnable = false;

  return m_pipelineContext->CreatePipeline(createInfo);
}

DescriptorSetLayout*
ShowBoxRenderPass::CreateDescriptorSetLayout() {
  std::vector<Marbas::DescriptorSetLayoutBinding> showBoxPipelinelayoutBinding;
  showBoxPipelinelayoutBinding.push_back(Marbas::DescriptorSetLayoutBinding{
      .bindingPoint = 0,
      .descriptorType = Marbas::DescriptorType::UNIFORM_BUFFER,
  });
  showBoxPipelinelayoutBinding.push_back(Marbas::DescriptorSetLayoutBinding{
      .bindingPoint = 0,
      .descriptorType = Marbas::DescriptorType::IMAGE,
  });
  return m_pipelineContext->CreateDescriptorSetLayout(showBoxPipelinelayoutBinding);
}

void
ShowBoxRenderPass::CreateDescriptorSet() {
  std::vector<Marbas::DescriptorPoolSize> descriptorSize = {
      Marbas::DescriptorPoolSize{
          .type = Marbas::DescriptorType::UNIFORM_BUFFER,
          .size = 1,
      },
      Marbas::DescriptorPoolSize{
          .type = Marbas::DescriptorType::IMAGE,
          .size = 1,
      },
  };
  m_descriptorPool = m_pipelineContext->CreateDescriptorPool(descriptorSize);

  m_descriptorSet = m_pipelineContext->CreateDescriptorSet(m_descriptorPool, m_descriptorSetLayout);
  m_pipelineContext->BindImage(BindImageInfo{
      .descriptorSet = m_descriptorSet,
      .bindingPoint = 0,
      .imageView = m_textureView,
      .sampler = m_sampler,
  });
  m_pipelineContext->BindBuffer(BindBufferInfo{
      .descriptorSet = m_descriptorSet,
      .descriptorType = Marbas::DescriptorType::UNIFORM_BUFFER,
      .bindingPoint = 0,
      .buffer = m_uniformBuffer,
      .offset = 0,
      .arrayElement = 0,
  });
}

void
ShowBoxRenderPass::RecordCommand(CommandBuffer* commandBuffer, uint32_t frameIndex) {
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
  commandBuffer->BeginPipeline(m_pipeline, m_frameBuffers[frameIndex], {{0, 0, 0, 1}, {1, 0}});
  commandBuffer->SetViewports({&viewportInfo, 1});
  commandBuffer->SetScissors({&scissorInfo, 1});
  commandBuffer->BindVertexBuffer(m_vertexBuffer);
  commandBuffer->BindDescriptorSet(m_pipeline, m_descriptorSet);
  commandBuffer->Draw(m_model.m_vertices.size(), 1, 0, 0);
  commandBuffer->EndPipeline(m_pipeline);
  commandBuffer->End();
}

}  // namespace Marbas
