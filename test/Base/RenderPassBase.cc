#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "RenderPassBase.hpp"

#include <stb_image.h>

#include <fstream>
#include <stdexcept>

namespace Marbas {

std::tuple<Marbas::Image*, uint32_t>
RenderPassBase::LoadImage(Marbas::BufferContext* bufferContext, const std::string& imagePath) {
  int texWidth, texHeight, texChannels;
  void* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }
  auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

  Marbas::Image2DDesc desc;

  Marbas::ImageCreateInfo imageCreateInfo;
  imageCreateInfo.width = texWidth;
  imageCreateInfo.height = texHeight;
  imageCreateInfo.mipMapLevel = mipLevels;
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
  stbi_image_free(pixels);

  return {image, mipLevels};
}

RenderPassBase::RenderPassBase(RHIFactory* rhiFactory)
    : m_bufferContext(rhiFactory->GetBufferContext()), m_pipelineContext(rhiFactory->GetPipelineContext()) {
  m_commandBuffer = m_bufferContext->CreateGraphicsCommandBuffer();
}

void
RenderPassBase::Render(std::span<Semaphore*> waitSemaphore, std::span<Semaphore*> signalSemaphore, Fence* fence,
                       uint32_t frameIndex) {
  RecordCommand(m_commandBuffer, frameIndex);
  m_commandBuffer->Submit(waitSemaphore, signalSemaphore, fence);
}

void
RenderPassBase::CreateFrameBuffer(std::vector<FrameBufferCreateInfo>& frameBufferCreateInfos) {
  for (auto& frameBufferCreateInfo : frameBufferCreateInfos) {
    frameBufferCreateInfo.pipeline = m_pipeline;
    m_frameBuffers.push_back(m_pipelineContext->CreateFrameBuffer(frameBufferCreateInfo));
  }
}

std::vector<char>
RenderPassBase::CreateShaderModule(const std::string& shaderPath) {
  std::ifstream file(shaderPath, std::ios::binary | std::ios::in);
  std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return content;
}

std::vector<char>
RenderPassBase::CreateShaderModule(RHIFactory* factory, const std::string& shaderPath) {
  auto pipelineContext = factory->GetPipelineContext();
  std::ifstream file(shaderPath, std::ios::binary | std::ios::in);
  std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return content;
}

}  // namespace Marbas
