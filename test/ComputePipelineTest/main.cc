#include <fstream>
#include <iostream>

#include "RHIFactory.hpp"

int
main(void) {
  int width = 800;
  int height = 600;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  auto bufferCtx = factory->GetBufferContext();
  auto pipelineCtx = factory->GetPipelineContext();

  auto commandPool = bufferCtx->CreateCommandPool(Marbas::CommandBufferUsage::COMPUTE);
  auto commandBuffer = bufferCtx->CreateCommandBuffer(commandPool);

  // create ssbo
  std::array<int, 5> ssboData = {1, 2, 3, 4, 5};
  auto* ssbo = bufferCtx->CreateBuffer(Marbas::BufferType::STORAGE_BUFFER, ssboData.data(),
                                       sizeof(int) * ssboData.size(), false);

  // create shader
  std::fstream file("compute.comp.glsl.spv", std::ios::in | std::ios::binary);
  std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  Marbas::ShaderStageCreateInfo shaderStageCreateInfo;
  shaderStageCreateInfo.stage = Marbas::ShaderType::COMPUTE_SHADER;
  shaderStageCreateInfo.interName = "main";
  shaderStageCreateInfo.code = content;

  // create descriptor set
  std::vector<Marbas::DescriptorPoolSize> descriptorPoolSize = {
      {Marbas::DescriptorType::STORAGE_BUFFER, 1},
  };
  std::vector<Marbas::DescriptorSetLayoutBinding> descriptorSetBindings{
      {.bindingPoint = 0, .descriptorType = Marbas::DescriptorType::STORAGE_BUFFER},
  };
  auto descriptorSetLayout = pipelineCtx->CreateDescriptorSetLayout(descriptorSetBindings);
  auto descriptorPool = pipelineCtx->CreateDescriptorPool(descriptorPoolSize, 1);
  auto descriptorSet = pipelineCtx->CreateDescriptorSet(descriptorPool, descriptorSetLayout);

  pipelineCtx->BindBuffer(Marbas::BindBufferInfo{
      .descriptorSet = descriptorSet,
      .descriptorType = Marbas::DescriptorType::STORAGE_BUFFER,
      .bindingPoint = 0,
      .buffer = ssbo,
      .offset = 0,
      .arrayElement = 0,
  });

  // create pipeline
  auto pipeline = pipelineCtx->CreatePipeline(Marbas::ComputePipelineCreateInfo{
      .computeShaderStage = shaderStageCreateInfo,
      .layout = descriptorSetLayout,
  });

  auto* fence = factory->CreateFence();
  factory->ResetFence(fence);

  commandBuffer->Begin();
  commandBuffer->BeginPipeline(pipeline, nullptr, {});
  commandBuffer->BindDescriptorSet(pipeline, descriptorSet);
  commandBuffer->Dispatch(1, 1, 1);
  commandBuffer->EndPipeline(pipeline);
  commandBuffer->End();
  commandBuffer->Submit({}, {}, fence);
  factory->WaitForFence(fence);

  // get data
  bufferCtx->GetBufferData(ssbo, ssboData.data());

  for (auto& num : ssboData) {
    std::cout << num << ", ";
  }
  std::cout << std::endl;
}
