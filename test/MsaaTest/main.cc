#define GLM_FORCE_RADIANS
#include <stb_image.h>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RHIFactory.hpp"
#include "RenderPassBase.hpp"

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
};

static const std::array vertices = {
    Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}}, Vertex{{0.5f, -0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{0.5f, 0.5f, -0.5f}, {0, 1, 0}},   Vertex{{0.5f, 0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, 0.5f, -0.5f}, {0, 1, 0}},  Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}},

    Vertex{{-0.5f, -0.5f, 0.5f}, {0, 1, 0}},  Vertex{{0.5f, -0.5f, 0.5f}, {0, 1, 0}},
    Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},    Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, 0.5f, 0.5f}, {0, 1, 0}},   Vertex{{-0.5f, -0.5f, 0.5f}, {0, 1, 0}},

    Vertex{{-0.5f, 0.5f, 0.5f}, {0, 1, 0}},   Vertex{{-0.5f, 0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}}, Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, -0.5f, 0.5f}, {0, 1, 0}},  Vertex{{-0.5f, 0.5f, 0.5f}, {0, 1, 0}},

    Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},    Vertex{{0.5f, 0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{0.5f, -0.5f, -0.5f}, {0, 1, 0}},  Vertex{{0.5f, -0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{0.5f, -0.5f, 0.5f}, {0, 1, 0}},   Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},

    Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}}, Vertex{{0.5f, -0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{0.5f, -0.5f, 0.5f}, {0, 1, 0}},   Vertex{{0.5f, -0.5f, 0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, -0.5f, 0.5f}, {0, 1, 0}},  Vertex{{-0.5f, -0.5f, -0.5f}, {0, 1, 0}},

    Vertex{{-0.5f, 0.5f, -0.5f}, {0, 1, 0}},  Vertex{{0.5f, 0.5f, -0.5f}, {0, 1, 0}},
    Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},    Vertex{{0.5f, 0.5f, 0.5f}, {0, 1, 0}},
    Vertex{{-0.5f, 0.5f, 0.5f}, {0, 1, 0}},   Vertex{{-0.5f, 0.5f, -0.5}, {0, 1, 0}},
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

Marbas::Image*
LoadImage(Marbas::BufferContext* bufferContext, const std::string& imagePath) {
  int texWidth, texHeight, texChannels;
  void* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  Marbas::Image2DDesc desc;

  Marbas::ImageCreateInfo imageCreateInfo;
  imageCreateInfo.width = texWidth;
  imageCreateInfo.height = texHeight;
  imageCreateInfo.mipMapLevel = 2;
  imageCreateInfo.format = Marbas::ImageFormat::RGBA;
  imageCreateInfo.imageDesc = desc;
  imageCreateInfo.usage = Marbas::ImageUsageFlags::SHADER_READ;

  auto image = bufferContext->CreateImage(imageCreateInfo);
  bufferContext->UpdateImage(Marbas::UpdateImageInfo{
      .image = image,
      .level = 0,
      .xOffset = 0,
      .yOffset = 0,
      .zOffset = 0,
      .width = texWidth,
      .height = texHeight,
      .depth = 1,
      .data = pixels,
      .dataSize = static_cast<uint32_t>(texWidth * texHeight * 4),
  });
  return image;
}

Marbas::Image*
CreateDepthBuffer(Marbas::BufferContext* bufferContext, uint32_t width, uint32_t height) {
  Marbas::ImageCreateInfo imageCreateInfo;
  imageCreateInfo.height = height;
  imageCreateInfo.width = width;
  imageCreateInfo.imageDesc = Marbas::Image2DDesc();
  imageCreateInfo.usage = Marbas::ImageUsageFlags::DEPTH_STENCIL;
  imageCreateInfo.mipMapLevel = 1;
  imageCreateInfo.format = Marbas::ImageFormat::DEPTH;
  imageCreateInfo.sampleCount = Marbas::SampleCount::BIT8;

  auto* image = bufferContext->CreateImage(imageCreateInfo);

  return image;
}

void
UpdateUniformBuffer(UniformBufferObject& ubo, uint32_t height, uint32_t width) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
}

int
main(void) {
  int width = 400;
  int height = 300;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  auto* pipelineContext = factory->GetPipelineContext();
  auto* bufferContext = factory->GetBufferContext();
  auto* swapchain = factory->GetSwapchain();

  auto* depthBuffer = CreateDepthBuffer(bufferContext, width, height);
  auto* depthBufferView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
      .image = depthBuffer,
      .type = Marbas::ImageViewType::TEXTURE2D,
      .baseLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });
  auto* colorAttachImage = bufferContext->CreateImage(Marbas::ImageCreateInfo{
      .width = static_cast<uint32_t>(width),
      .height = static_cast<uint32_t>(height),
      .format = swapchain->imageFormat,
      .sampleCount = Marbas::SampleCount::BIT8,
      .mipMapLevel = 1,
      .usage = Marbas::ImageUsageFlags::COLOR_RENDER_TARGET,
      .imageDesc = Marbas::Image2DDesc{},
  });
  auto* colorAttachmentView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
      .image = colorAttachImage,
      .type = Marbas::ImageViewType::TEXTURE2D,
      .baseLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });

  auto sampler = pipelineContext->CreateSampler(Marbas::SamplerCreateInfo{
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

  // create uniform buffer
  UniformBufferObject ubo;
  ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 10.0f);

  auto* uniformbuffer =
      bufferContext->CreateBuffer(Marbas::BufferType::UNIFORM_BUFFER, &ubo, sizeof(UniformBufferObject), true);

  // create descriptor pool
  Marbas::DescriptorSetArgument argument;
  argument.Bind(0, Marbas::DescriptorType::UNIFORM_BUFFER);

  // create descriptorSet
  auto descriptorSet = pipelineContext->CreateDescriptorSet(argument);
  pipelineContext->BindBuffer(Marbas::BindBufferInfo{
      .descriptorSet = descriptorSet,
      .descriptorType = Marbas::DescriptorType::UNIFORM_BUFFER,
      .bindingPoint = 0,
      .buffer = uniformbuffer,
      .offset = 0,
      .arrayElement = 0,
  });

  // render target desc and blend
  Marbas::RenderTargetDesc renderTargetDesc{
      .colorAttachments = {Marbas::ColorTargetDesc{
          .initAction = Marbas::AttachmentInitAction::CLEAR,
          .finalAction = Marbas::AttachmentFinalAction::DISCARD,
          .usage = Marbas::ImageUsageFlags::COLOR_RENDER_TARGET,
          .sampleCount = Marbas::SampleCount::BIT8,
          .format = swapchain->imageFormat,
      }},
      .depthAttachments =
          Marbas::DepthTargetDesc{
              .initAction = Marbas::AttachmentInitAction::CLEAR,
              .finalAction = Marbas::AttachmentFinalAction::DISCARD,
              .usage = Marbas::ImageUsageFlags::DEPTH_STENCIL,
              .sampleCount = Marbas::SampleCount::BIT8,
          },
      .resolveAttachments = {Marbas::ResolveTargetDesc{
          .initAction = Marbas::AttachmentInitAction::CLEAR,
          .finalAction = Marbas::AttachmentFinalAction::PRESENT,
          .usage = Marbas::ImageUsageFlags::COLOR_RENDER_TARGET,
          .format = swapchain->imageFormat,
      }},
  };

  Marbas::BlendAttachment renderTargetBlendAttachment;
  renderTargetBlendAttachment.blendEnable = false;

  std::vector<Marbas::ShaderStageCreateInfo> shaderStageCreateInfos;
  shaderStageCreateInfos.emplace_back("shader.vert.spv", Marbas::ShaderType::VERTEX_SHADER, "main");
  shaderStageCreateInfos.emplace_back("shader.frag.spv", Marbas::ShaderType::FRAGMENT_SHADER, "main");

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

  // vertex input layout
  Marbas::InputElementDesc posAttribute, colorAttribute;
  Marbas::InputElementView elementView;
  posAttribute.binding = 0;
  posAttribute.attribute = 0;
  posAttribute.format = Marbas::ElementType::R32G32B32_SFLOAT;
  posAttribute.offset = offsetof(Vertex, pos);
  posAttribute.instanceStepRate = 0;

  colorAttribute.binding = 0;
  colorAttribute.attribute = 1;
  colorAttribute.format = Marbas::ElementType::R32G32B32_SFLOAT;
  colorAttribute.offset = offsetof(Vertex, color);

  elementView.binding = 0;
  elementView.inputClass = Marbas::VertexInputClass::VERTEX;
  elementView.stride = sizeof(Vertex);

  Marbas::GraphicsPipeLineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.vertexInputLayout.elementDesc = {posAttribute, colorAttribute};
  pipelineCreateInfo.vertexInputLayout.viewDesc = {elementView};
  pipelineCreateInfo.outputRenderTarget = renderTargetDesc;
  pipelineCreateInfo.shaderStageCreateInfo = shaderStageCreateInfos;
  pipelineCreateInfo.rasterizationInfo.frontFace = Marbas::FrontFace::CCW;
  pipelineCreateInfo.multisampleCreateInfo.rasterizationSamples = Marbas::SampleCount::BIT8;
  pipelineCreateInfo.layout = {argument};
  pipelineCreateInfo.depthStencilInfo.depthTestEnable = true;
  pipelineCreateInfo.depthStencilInfo.stencilTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthBoundsTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthWriteEnable = true;
  pipelineCreateInfo.inputAssemblyState.topology = Marbas::PrimitiveTopology::TRIANGLE;
  pipelineCreateInfo.blendInfo.attachments.push_back(renderTargetBlendAttachment);

  auto pipeline = pipelineContext->CreatePipeline(pipelineCreateInfo);

  // frame buffer
  std::vector<Marbas::FrameBuffer*> frameBuffers;
  for (int i = 0; i < swapchain->imageViews.size(); i++) {
    Marbas::FrameBufferCreateInfo createInfo;
    createInfo.height = height;
    createInfo.width = width;
    createInfo.layer = 1;
    createInfo.pipeline = pipeline;
    createInfo.attachments.colorAttachments = {colorAttachmentView};
    createInfo.attachments.depthAttachment = depthBufferView;
    createInfo.attachments.resolveAttachments = {swapchain->imageViews[i]};
    frameBuffers.push_back(pipelineContext->CreateFrameBuffer(createInfo));
  }

  // vertex buffer
  auto VBOSize = vertices.size() * sizeof(Vertex);
  auto* vertexBuffer = bufferContext->CreateBuffer(Marbas::BufferType::VERTEX_BUFFER, vertices.data(), VBOSize, false);

  // command buffer
  auto imageCount = swapchain->imageViews.size();
  auto* commandBuffer = bufferContext->CreateGraphicsCommandBuffer();
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

    // destroy depthBuffer and recreate
    bufferContext->DestroyImage(depthBuffer);
    bufferContext->DestroyImageView(depthBufferView);
    bufferContext->DestroyImage(colorAttachImage);
    bufferContext->DestroyImageView(colorAttachmentView);

    depthBuffer = CreateDepthBuffer(bufferContext, width, height);
    depthBufferView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
        .image = depthBuffer,
        .type = Marbas::ImageViewType::TEXTURE2D,
        .baseLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    });

    colorAttachImage = bufferContext->CreateImage(Marbas::ImageCreateInfo{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .format = swapchain->imageFormat,
        .sampleCount = Marbas::SampleCount::BIT8,
        .mipMapLevel = 1,
        .usage = Marbas::ImageUsageFlags::COLOR_RENDER_TARGET,
        .imageDesc = Marbas::Image2DDesc{},
    });
    colorAttachmentView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
        .image = colorAttachImage,
        .type = Marbas::ImageViewType::TEXTURE2D,
        .baseLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    });

    // recreate framebuffer
    for (int i = 0; i < frameBuffers.size(); i++) {
      pipelineContext->DestroyFrameBuffer(frameBuffers[i]);

      Marbas::FrameBufferCreateInfo createInfo;
      createInfo.height = height;
      createInfo.width = width;
      createInfo.layer = 1;
      createInfo.pipeline = pipeline;

      createInfo.attachments.colorAttachments = {colorAttachmentView};
      createInfo.attachments.depthAttachment = depthBufferView;
      createInfo.attachments.resolveAttachments = {swapchain->imageViews[i]};
      frameBuffers[i] = pipelineContext->CreateFrameBuffer(createInfo);
    }

    // viewport and scissor
    viewportInfo.width = static_cast<float>(width);
    viewportInfo.height = static_cast<float>(height);
    scissorInfo.width = width;
    scissorInfo.height = height;
  };

  uint32_t frameIndex = 0;
  bool needResize = false;
  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
    factory->ResetFence(fence);

    if (needResize) {
      glfwGetFramebufferSize(glfwWindow, &width, &height);
      if (width > 0 && height > 0) {
        onResize(width, height);
        needResize = false;
        continue;
      }
    }

    int w, h;
    glfwGetWindowSize(glfwWindow, &w, &h);
    const bool is_minimized = (w <= 0 || h <= 0);
    if (is_minimized) continue;

    auto nextImage = factory->AcquireNextImage(swapchain, aviableSemaphore[frameIndex]);
    if (nextImage == -1) {
      needResize = true;
      continue;
    }

    std::array<Marbas::ViewportInfo, 1> viewportInfos = {viewportInfo};
    std::array<Marbas::ScissorInfo, 1> scissorInfos = {scissorInfo};

    UpdateUniformBuffer(ubo, height, width);
    bufferContext->UpdateBuffer(uniformbuffer, &ubo, sizeof(ubo), 0);

    commandBuffer->Begin();
    commandBuffer->BeginPipeline(pipeline, frameBuffers[nextImage],
                                 {
                                     Marbas::ClearValue(std::array<float, 4>{0, 0, 0, 1}),
                                     Marbas::ClearValue(std::array<float, 2>{1.0, 0}),
                                     Marbas::ClearValue(std::array<float, 4>{0, 0, 0, 1}),
                                 });
    commandBuffer->SetViewports(viewportInfos);
    commandBuffer->SetScissors(scissorInfos);
    commandBuffer->BindVertexBuffer(vertexBuffer);
    commandBuffer->BindDescriptorSet(pipeline, {descriptorSet});
    commandBuffer->Draw(vertices.size(), 1, 0, 0);
    commandBuffer->EndPipeline(pipeline);
    commandBuffer->End();

    commandBuffer->Submit({aviableSemaphore.begin() + frameIndex, 1}, {waitSemaphore.begin() + frameIndex, 1}, fence);

    auto presentResult = factory->Present(swapchain, {waitSemaphore.begin() + frameIndex, 1}, nextImage);
    factory->WaitForFence(fence);
    if (presentResult == -1) {
      needResize = true;
      continue;
    }
    frameIndex = (frameIndex + 1) % imageCount;
  }

  factory->WaitIdle();

  for (auto* framebuffer : frameBuffers) {
    pipelineContext->DestroyFrameBuffer(framebuffer);
  }
  pipelineContext->DestroyPipeline(pipeline);
  pipelineContext->DestroySampler(sampler);

  bufferContext->DestroyBuffer(uniformbuffer);
  bufferContext->DestroyBuffer(vertexBuffer);
  bufferContext->DestroyCommandBuffer(commandBuffer);
  bufferContext->DestroyImage(depthBuffer);
  bufferContext->DestroyImage(colorAttachImage);
  bufferContext->DestroyImageView(depthBufferView);
  bufferContext->DestroyImageView(colorAttachmentView);

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
