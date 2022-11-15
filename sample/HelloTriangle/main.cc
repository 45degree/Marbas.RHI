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
  auto* bufferContext = factory->GetBufferContext();
  auto* swapchain = factory->GetSwapchain();
  auto* vertexShader = pipelineContext->CreateShaderModule("shader.vert.spv");
  auto* fragShader = pipelineContext->CreateShaderModule("shader.frag.spv");

  // render target desc and blend
  Marbas::RenderTargetDesc renderTargetDesc{
      .isClear = true,
      .isDepth = false,
      .isPresent = true,
      .format = swapchain->imageFormat,
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
  viewportInfo.width = static_cast<float>(width);
  viewportInfo.height = static_cast<float>(height);
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
  pipelineCreateInfo.inputAssemblyState.topology = Marbas::PrimitiveTopology::TRIANGLE;
  pipelineCreateInfo.blendInfo.attachments.push_back(renderTargetBlendAttachment);

  auto* pipeline = pipelineContext->CreatePipeline(pipelineCreateInfo);

  // frame buffer
  std::vector<Marbas::FrameBuffer*> frameBuffers;
  for (int i = 0; i < swapchain->imageViews.size(); i++) {
    Marbas::FrameBufferCreateInfo createInfo;
    createInfo.height = height;
    createInfo.width = width;
    createInfo.layer = 1;
    createInfo.pieline = pipeline;
    createInfo.attachments = std::span(swapchain->imageViews.begin() + i, 1);
    frameBuffers.push_back(pipelineContext->CreateFrameBuffer(createInfo));
  }

  // command buffer
  auto imageCount = swapchain->imageViews.size();
  auto* commandPool = bufferContext->CreateCommandPool(Marbas::CommandBufferUsage::GRAPHICS);
  auto* commandBuffer = bufferContext->CreateCommandBuffer(commandPool);
  std::vector<Marbas::Semaphore*> aviableSemaphore;
  std::vector<Marbas::Semaphore*> waitSemaphore;
  for (int i = 0; i < imageCount; i++) {
    aviableSemaphore.push_back(factory->CreateGPUSemaphore());
    waitSemaphore.push_back(factory->CreateGPUSemaphore());
  }

  Marbas::Fence* fence = factory->CreateFence();

  auto onResize = [&](int width, int height) {
    factory->WaitIdle();
    factory->RecreateSwapchain(swapchain, width, height);
    // recreate framebuffer
    for (int i = 0; i < frameBuffers.size(); i++) {
      pipelineContext->DestroyFrameBuffer(frameBuffers[i]);

      Marbas::FrameBufferCreateInfo createInfo;
      createInfo.height = height;
      createInfo.width = width;
      createInfo.layer = 1;
      createInfo.pieline = pipeline;
      createInfo.attachments = std::span(swapchain->imageViews.begin() + i, 1);
      frameBuffers[i] = pipelineContext->CreateFrameBuffer(createInfo);
    }

    // viewport and scissor
    viewportInfo.width = static_cast<float>(width);
    viewportInfo.height = static_cast<float>(height);
    scissorInfo.width = width;
    scissorInfo.height = height;
  };

  uint32_t frameIndex = 0;
  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
    factory->ResetFence(fence);
    auto nextImage = factory->AcquireNextImage(swapchain, aviableSemaphore[frameIndex]);
    if (nextImage == -1) {
      glfwGetFramebufferSize(glfwWindow, &width, &height);
      onResize(width, height);
      continue;
    }

    std::array<Marbas::ViewportInfo, 1> viewportInfos = {viewportInfo};
    std::array<Marbas::ScissorInfo, 1> scissorInfos = {scissorInfo};

    commandBuffer->Begin();
    commandBuffer->BeginPipeline(pipeline, frameBuffers[nextImage], {1, 1, 1, 1});
    commandBuffer->SetViewports(viewportInfos);
    commandBuffer->SetScissors(scissorInfos);
    commandBuffer->Draw(3, 1, 0, 0);
    commandBuffer->EndPipeline(pipeline);
    commandBuffer->End();

    commandBuffer->Submit({aviableSemaphore.begin() + frameIndex, 1}, {waitSemaphore.begin() + frameIndex, 1}, fence);

    if (factory->Present(swapchain, {waitSemaphore.begin() + frameIndex, 1}, nextImage) == -1) {
      glfwGetFramebufferSize(glfwWindow, &width, &height);
      onResize(width, height);
    }
    factory->WaitForFence(fence);
    frameIndex = (frameIndex + 1) % imageCount;
  }

  factory->WaitIdle();
  pipelineContext->DestroyShaderModule(vertexShader);
  pipelineContext->DestroyShaderModule(fragShader);
  for (auto* framebuffer : frameBuffers) {
    pipelineContext->DestroyFrameBuffer(framebuffer);
  }
  pipelineContext->DestroyPipeline(pipeline);

  bufferContext->DestroyCommandBuffer(commandPool, commandBuffer);
  bufferContext->DestroyCommandPool(commandPool);

  factory->DestroyFence(fence);
  for (auto* semaphore : waitSemaphore) {
    factory->DestroyGPUSemaphore(semaphore);
  }
  for (auto* semaphore : aviableSemaphore) {
    factory->DestroyGPUSemaphore(semaphore);
  }
  factory->Quit();

  return 0;
}
