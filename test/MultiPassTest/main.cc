#include "GLFW/glfw3.h"
#include "Model.hpp"
#include "RHIFactory.hpp"
#include "ShowBoxRenderPass.hpp"
#include "ShowPlaneRenderPass.hpp"
#include "ShowScreenRenderPass.hpp"

Marbas::Image*
CreateDepthBuffer(Marbas::BufferContext* bufferContext, uint32_t width, uint32_t height) {
  Marbas::ImageCreateInfo imageCreateInfo;
  imageCreateInfo.height = height;
  imageCreateInfo.width = width;
  imageCreateInfo.imageDesc = Marbas::Image2DDesc();
  imageCreateInfo.usage = Marbas::ImageUsageFlags::DEPTH_STENCIL;
  imageCreateInfo.mipMapLevel = 1;
  imageCreateInfo.format = Marbas::ImageFormat::DEPTH;
  imageCreateInfo.sampleCount = Marbas::SampleCount::BIT1;

  auto* image = bufferContext->CreateImage(imageCreateInfo);

  return image;
}

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);

  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);
  factory->Init(glfwWindow, width, height);

  auto* swapChain = factory->GetSwapchain();
  auto* bufferContext = factory->GetBufferContext();
  auto* pipelineContext = factory->GetPipelineContext();

  // create a color attachments
  auto* image = bufferContext->CreateImage(Marbas::ImageCreateInfo{
      .width = static_cast<uint32_t>(width),
      .height = static_cast<uint32_t>(height),
      .format = swapChain->imageFormat,
      .sampleCount = Marbas::SampleCount::BIT1,
      .mipMapLevel = 1,
      .usage = Marbas::ImageUsageFlags::COLOR_RENDER_TARGET | Marbas::ImageUsageFlags::SHADER_READ,
      .imageDesc = Marbas::Image2DDesc{},
  });
  auto* imageView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
      .image = image,
      .type = Marbas::ImageViewType::TEXTURE2D,
      .baseLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });

  // swapchain semaphores
  auto imageCount = swapChain->images.size();
  std::vector<Marbas::Semaphore*> aviableSemaphores(imageCount);
  std::vector<Marbas::Semaphore*> waitSemaphores(imageCount);
  for (int i = 0; i < imageCount; i++) {
    aviableSemaphores[i] = factory->CreateGPUSemaphore();
    waitSemaphores[i] = factory->CreateGPUSemaphore();
  }

  // create frameBuffer
  auto depthBuffer = CreateDepthBuffer(bufferContext, width, height);
  auto* depthBufferView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
      .image = depthBuffer,
      .type = Marbas::ImageViewType::TEXTURE2D,
      .baseLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });
  std::vector<Marbas::FrameBufferCreateInfo> frameBufferCreateInfos;
  for (int i = 0; i < swapChain->imageViews.size(); i++) {
    Marbas::FrameBufferCreateInfo createInfo;
    createInfo.height = height;
    createInfo.width = width;
    createInfo.layer = 1;
    createInfo.attachments.colorAttachments = {imageView};
    createInfo.attachments.depthAttachment = depthBufferView;
    frameBufferCreateInfos.push_back(createInfo);
  }

  // create commandBuffer
  auto commandPool = bufferContext->CreateCommandPool(Marbas::CommandBufferUsage::TRANSFER);
  auto* commandBuffer = bufferContext->CreateCommandBuffer(commandPool);

  {
    Marbas::ShowBoxRenderPass showBoxRenderPass(factory.get());
    Marbas::ShowPlaneRenderPass showPlaneRenderPass(factory.get());
    Marbas::ShowScreenRenderPass showScreenRenderPass(factory.get(), imageView);
    showBoxRenderPass.CreateFrameBuffer(frameBufferCreateInfos);
    showPlaneRenderPass.CreateFrameBuffer(frameBufferCreateInfos);

    for (int i = 0; i < frameBufferCreateInfos.size(); i++) {
      auto& createInfo = frameBufferCreateInfos[i];
      createInfo.attachments.colorAttachments = {swapChain->imageViews[i]};
      createInfo.attachments.depthAttachment = nullptr;
    }
    showScreenRenderPass.CreateFrameBuffer(frameBufferCreateInfos);

    uint32_t currentFrame = 0;
    Marbas::Semaphore* showBoxRenderPassSemaphore = factory->CreateGPUSemaphore();
    Marbas::Semaphore* showPlaneSemaphore = factory->CreateGPUSemaphore();
    auto* fence = factory->CreateFence();
    while (!glfwWindowShouldClose(glfwWindow)) {
      glfwPollEvents();
      factory->ResetFence(fence);
      auto nextImage = factory->AcquireNextImage(swapChain, aviableSemaphores[currentFrame]);

      showBoxRenderPass.Render({aviableSemaphores.begin() + currentFrame, 1}, {&showBoxRenderPassSemaphore, 1}, nullptr,
                               currentFrame);
      showPlaneRenderPass.Render({&showBoxRenderPassSemaphore, 1}, {&showPlaneSemaphore, 1}, nullptr, currentFrame);

      showScreenRenderPass.Render({&showPlaneSemaphore, 1}, {waitSemaphores.begin() + currentFrame, 1}, fence,
                                  nextImage);
      factory->Present(swapChain, std::span(waitSemaphores.begin() + currentFrame, 1), nextImage);
      factory->WaitForFence(fence);
      currentFrame = (currentFrame + 1) % imageCount;
    }
    factory->WaitIdle();
    factory->DestroyFence(fence);
    factory->DestroyGPUSemaphore(showBoxRenderPassSemaphore);
    factory->DestroyGPUSemaphore(showPlaneSemaphore);
  }

  for (int i = 0; i < aviableSemaphores.size(); i++) {
    factory->DestroyGPUSemaphore(aviableSemaphores[i]);
    factory->DestroyGPUSemaphore(waitSemaphores[i]);
  }

  bufferContext->DestroyImage(image);
  bufferContext->DestroyImageView(imageView);
  bufferContext->DestroyImage(depthBuffer);
  bufferContext->DestroyImageView(depthBufferView);
  bufferContext->DestroyCommandBuffer(commandPool, commandBuffer);
  bufferContext->DestroyCommandPool(commandPool);

  factory->Quit();
}
