#include "RHIFactory.hpp"

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  auto* pipelineContext = factory->GetPipelineContext();

  auto* vertexShader = pipelineContext->CreateShaderModule("shader.vert.spv");
  auto* fragShader = pipelineContext->CreateShaderModule("shader.frag.spv");

  // render target desc and blend
  Marbas::RenderTargetDesc renderTargetDesc{
      .isClear = true,
      .isDepth = false,
      .format = Marbas::ImageFormat::RGBA,
  };

  Marbas::BlendAttachment renderTargetBlendAttachment;
  renderTargetBlendAttachment.blendEnable = false;

  std::vector<Marbas::ShaderStageCreateInfo> shaderStageCreateInfos;
  shaderStageCreateInfos.push_back(Marbas::ShaderStageCreateInfo{
      .stage = Marbas::ShaderType::VERTEX_SHADER,
      .shaderModule = vertexShader,
      .interName = "main",
  });
  shaderStageCreateInfos.push_back(Marbas::ShaderStageCreateInfo{
      .stage = Marbas::ShaderType::FRAGMENT_SHADER,
      .shaderModule = fragShader,
      .interName = "main",
  });

  Marbas::ViewportInfo viewportInfo;
  viewportInfo.x = 0;
  viewportInfo.y = 0;
  viewportInfo.width = width;
  viewportInfo.height = height;
  viewportInfo.minDepth = 0;
  viewportInfo.maxDepth = 1;

  Marbas::ScissorInfo scissorInfo;
  scissorInfo.x = 0;
  scissorInfo.y = 0;
  scissorInfo.width = width;
  scissorInfo.height = height;

  // multi sample

  Marbas::GraphicsPipeLineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.outputRenderTarget.push_back(renderTargetDesc);
  pipelineCreateInfo.shaderStageCreateInfo = shaderStageCreateInfos;
  pipelineCreateInfo.multisampleCreateInfo.rasterizationSamples = Marbas::SampleCount::BIT1;
  pipelineCreateInfo.depthStencilInfo.depthTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.stencilTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthBoundsTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthWriteEnable = false;
  pipelineCreateInfo.viewportStateCreateInfo.viewportInfos.push_back(viewportInfo);
  pipelineCreateInfo.viewportStateCreateInfo.scissorInfos.push_back(scissorInfo);
  pipelineCreateInfo.inputAssemblyState.topology = Marbas::PrimitiveTopology::TRIANGLE;
  pipelineCreateInfo.blendInfo.attachments.push_back(renderTargetBlendAttachment);

  auto* pipeline = pipelineContext->CreatePipeline(pipelineCreateInfo);

  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
  }

  pipelineContext->DestroyShaderModule(vertexShader);
  pipelineContext->DestroyShaderModule(fragShader);
  pipelineContext->DestroyPipeline(pipeline);

  return 0;
}
