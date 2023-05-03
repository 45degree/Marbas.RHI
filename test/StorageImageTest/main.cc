#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

  auto* storageImage = bufferContext->CreateImage(Marbas::ImageCreateInfo{
      .width = 300,
      .height = 300,
      .format = Marbas::ImageFormat::RGBA,
      .sampleCount = Marbas::SampleCount::BIT1,
      .mipMapLevel = 1,
      .usage = Marbas::ImageUsageFlags::STORAGE,
      .imageDesc = Marbas::Image2DDesc(),
  });
  auto* imageView = bufferContext->CreateImageView(Marbas::ImageViewCreateInfo{
      .image = storageImage,
      .type = Marbas::ImageViewType::TEXTURE2D,
      .baseLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  });

  // create descriptor pool
  Marbas::DescriptorSetArgument argument;
  argument.Bind(0, Marbas::DescriptorType::STORAGE_IMAGE);

  // create descriptorSet
  auto descriptorSet = pipelineContext->CreateDescriptorSet(argument);
  pipelineContext->BindStorageImage(Marbas::BindStorageImageInfo{
      .descriptorSet = descriptorSet,
      .bindingPoint = 0,
      .imageView = imageView,
  });

  std::vector<Marbas::ShaderStageCreateInfo> shaderStageCreateInfos;
  shaderStageCreateInfos.emplace_back("shader.vert.spv", Marbas::ShaderType::VERTEX_SHADER);
  shaderStageCreateInfos.emplace_back("shader.frag.spv", Marbas::ShaderType::FRAGMENT_SHADER);

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

  // test

  Marbas::GraphicsPipeLineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.outputRenderTarget = {};
  pipelineCreateInfo.shaderStageCreateInfo = shaderStageCreateInfos;
  pipelineCreateInfo.multisampleCreateInfo.rasterizationSamples = Marbas::SampleCount::BIT1;
  pipelineCreateInfo.layout = {argument};
  pipelineCreateInfo.depthStencilInfo.depthTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.stencilTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthBoundsTestEnable = false;
  pipelineCreateInfo.depthStencilInfo.depthWriteEnable = false;
  pipelineCreateInfo.inputAssemblyState.topology = Marbas::PrimitiveTopology::TRIANGLE;
  pipelineCreateInfo.blendInfo.attachments = {};

  auto pipeline = pipelineContext->CreatePipeline(pipelineCreateInfo);

  // frame buffer
  Marbas::FrameBufferCreateInfo createInfo;
  createInfo.height = height;
  createInfo.width = width;
  createInfo.layer = 1;
  createInfo.pipeline = pipeline;
  createInfo.attachments.colorAttachments = {};
  auto* frameBuffer = pipelineContext->CreateFrameBuffer(createInfo);

  // command buffer
  auto* commandBuffer = bufferContext->CreateGraphicsCommandBuffer();

  Marbas::Fence* fence = factory->CreateFence();

  commandBuffer->Begin();
  commandBuffer->BeginPipeline(pipeline, frameBuffer, {});
  commandBuffer->SetViewports({&viewportInfo, 1});
  commandBuffer->SetScissors({&scissorInfo, 1});
  commandBuffer->BindDescriptorSet(pipeline, {descriptorSet});
  commandBuffer->Draw(6, 1, 0, 0);
  commandBuffer->EndPipeline(pipeline);
  commandBuffer->End();

  commandBuffer->Submit({}, {}, fence);

  factory->WaitForFence(fence);

  Marbas::ImageSubresourceDesc subResource{
      .image = storageImage,
      .baseArrayLayer = 0,
      .layerCount = 1,
      .mipmapLevel = 0,
  };
  auto bufferSize = factory->GetBufferContext()->GetImageSubresourceSize(subResource);

  std::vector<char> data;
  data.resize(bufferSize);
  factory->GetBufferContext()->GetImageData(subResource, data.data());

  auto resheight = std::max(1U, storageImage->height >> subResource.mipmapLevel);
  auto reswidth = std::max(1U, storageImage->width >> subResource.mipmapLevel);
  auto resDepth = std::max(1U, storageImage->depth >> subResource.mipmapLevel);
  stbi_write_jpg("texture2.jpg", reswidth, resheight, 4, data.data(), 100);

  factory->WaitIdle();

  bufferContext->DestroyImageView(imageView);
  bufferContext->DestroyImage(storageImage);
  bufferContext->DestroyCommandBuffer(commandBuffer);

  pipelineContext->DestroyFrameBuffer(frameBuffer);
  pipelineContext->DestroyPipeline(pipeline);
  factory->DestroyFence(fence);
  factory->Quit();
  return 0;
}
