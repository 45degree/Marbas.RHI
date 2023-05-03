#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <iostream>
#include <stdexcept>

#include "RHIFactory.hpp"

std::tuple<Marbas::Image*, uint32_t>
LoadImage(Marbas::BufferContext* bufferContext, const std::string& imagePath) {
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

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  auto [image, lod] = LoadImage(factory->GetBufferContext(), "texture.jpg");
  factory->GetBufferContext()->GenerateMipmap(image, lod);

  Marbas::ImageSubresourceDesc subResource{
      .image = image,
      .baseArrayLayer = 0,
      .layerCount = 1,
      .mipmapLevel = 5,
  };
  auto bufferSize = factory->GetBufferContext()->GetImageSubresourceSize(subResource);
  std::cout << "buffer size:" << bufferSize << std::endl;

  std::vector<char> data;
  data.resize(bufferSize);
  factory->GetBufferContext()->GetImageData(subResource, data.data());

  auto resheight = std::max(1U, image->height >> subResource.mipmapLevel);
  auto reswidth = std::max(1U, image->width >> subResource.mipmapLevel);
  auto resDepth = std::max(1U, image->depth >> subResource.mipmapLevel);
  stbi_write_jpg("texture2.jpg", reswidth, resheight, 4, data.data(), 100);
}
