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

  // render target desc
  Marbas::RenderTargetDesc renderTargetDesc{
      .isClear = true,
      .isDepth = false,
      .format = Marbas::ImageFormat::RGBA,
  };

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

  Marbas::GraphicsPipeLineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.outputRenderTarget.push_back(renderTargetDesc);
  pipelineCreateInfo.shaderStageCreateInfo = shaderStageCreateInfos;

  auto* pipeline = pipelineContext->CreatePipeline(pipelineCreateInfo);

  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
  }

  pipelineContext->DestroyShaderModule(vertexShader);
  pipelineContext->DestroyShaderModule(fragShader);
  pipelineContext->DestroyPipeline(pipeline);

  return 0;
}
