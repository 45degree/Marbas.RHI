#include "GLFW/glfw3.h"
#include "Model.hpp"
#include "RHIFactory.hpp"
#include "ShowBoxRenderPass.hpp"
#include "ShowPlaneRenderPass.hpp"

Marbas::Image*
CreateDepthBuffer(Marbas::BufferContext* bufferContext, uint32_t width, uint32_t height) {
  Marbas::ImageCreateInfo imageCreateInfo;
  imageCreateInfo.height = height;
  imageCreateInfo.width = width;
  imageCreateInfo.imageDesc = Marbas::Image2DDesc();
  imageCreateInfo.usage = Marbas::ImageUsageFlags::DEPTH;
  imageCreateInfo.mipMapLevel = 1;
  imageCreateInfo.format = Marbas::ImageFormat::DEPTH;
  imageCreateInfo.sampleCount = Marbas::SampleCount::BIT1;

  auto* image = bufferContext->CreateImage(imageCreateInfo);
  bufferContext->ConvertImageState(image, Marbas::ImageState::UNDEFINED, Marbas::ImageState::DEPTH);

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
    createInfo.attachments.colorAttachments = std::span(swapChain->imageViews.begin() + i, 1);
    createInfo.attachments.depthAttachment = depthBufferView;
    frameBufferCreateInfos.push_back(createInfo);
  }

  // create commandBuffer
  Marbas::ShowBoxRenderPass showBoxRenderPass(factory.get());
  Marbas::ShowPlaneRenderPass showPlaneRenderPass(factory.get());
  showBoxRenderPass.CreateFrameBuffer(frameBufferCreateInfos);
  showPlaneRenderPass.CreateFrameBuffer(frameBufferCreateInfos);

  uint32_t currentFrame = 0;
  Marbas::Semaphore* showBoxRenderPassSemaphore = factory->CreateGPUSemaphore();

  auto* fence = factory->CreateFence();
  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
    factory->ResetFence(fence);
    auto nextImage = factory->AcquireNextImage(swapChain, aviableSemaphores[currentFrame]);

    showBoxRenderPass.Render({aviableSemaphores.begin() + currentFrame, 1}, {&showBoxRenderPassSemaphore, 1}, nullptr,
                             currentFrame);
    // showPlaneRenderPass.Render({aviableSemaphores.begin() + currentFrame, 1},
    //                            {waitSemaphores.begin() + currentFrame, 1}, fence, currentFrame);
    // factory->WaitIdle();
    showPlaneRenderPass.Render({&showBoxRenderPassSemaphore, 1}, {waitSemaphores.begin() + currentFrame, 1}, fence,
                               currentFrame);

    factory->Present(swapChain, std::span(waitSemaphores.begin() + currentFrame, 1), nextImage);
    factory->WaitForFence(fence);
    currentFrame = (currentFrame + 1) % imageCount;
  }
}
