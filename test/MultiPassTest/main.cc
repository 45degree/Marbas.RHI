#include "GLFW/glfw3.h"
#include "Model.hpp"
#include "RHIFactory.hpp"

const static std::array planeVertices = {
    // positions          // texture Coords
    Marbas::Vertex{{5.0f, -0.5f, 5.0f}, {2.0f, 0.0f}},    //
    Marbas::Vertex{{-5.0f, -0.5f, 5.0f}, {0.0f, 0.0f}},   //
    Marbas::Vertex{{-5.0f, -0.5f, -5.0f}, {0.0f, 2.0f}},  //
                                                          //
    Marbas::Vertex{{5.0f, -0.5f, 5.0f}, {2.0f, 0.0f}},    //
    Marbas::Vertex{{-5.0f, -0.5f, -5.0f}, {0.0f, 2.0f}},  //
    Marbas::Vertex{{5.0f, -0.5f, -5.0f}, {2.0f, 2.0f}}    //
};

const static std::array cubeVertices = {
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  //
    Marbas::Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},   //
    Marbas::Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
    Marbas::Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
    Marbas::Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   //
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  //
                                                          //
    Marbas::Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
    Marbas::Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},     //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},     //
    Marbas::Vertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},    //
    Marbas::Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
                                                          //
    Marbas::Vertex{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    //
    Marbas::Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},   //
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
    Marbas::Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
    Marbas::Vertex{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    //
                                                          //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
    Marbas::Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
    Marbas::Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},   //
    Marbas::Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},   //
    Marbas::Vertex{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},    //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
                                                          //
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
    Marbas::Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},   //
    Marbas::Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
    Marbas::Vertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    //
    Marbas::Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   //
    Marbas::Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  //
                                                          //
    Marbas::Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   //
    Marbas::Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},    //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
    Marbas::Vertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},     //
    Marbas::Vertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},    //
    Marbas::Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}    //
};

struct CameraInfo {
  glm::mat4 model = glm::mat4(1.0);
  glm::mat4 view = glm::lookAt(glm::vec3(5, 5, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 project = glm::perspective(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.);
};

static CameraInfo cameraInfo;
constexpr static std::string_view texturePath = "Test/metal.png";
constexpr static std::string_view vertexShaderPath = "Test/showBox.vert.glsl";
constexpr static std::string_view fragmentShaderPath = "Test/showBox.frag.glsl";

int
main(void) {
  int width = 400;
  int height = 300;

  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);
  auto* swapChain = factory->GetSwapchain();
  auto* bufferContext = factory->GetBufferContext();
  auto* pipelineContext = factory->GetPipelineContext();

  auto imageCount = swapChain->images.size();

  // read shader
  auto* showBoxVertModule = pipelineContext->CreateShaderModule("showBox.vert.glsl");
  auto* showBoxFragModule = pipelineContext->CreateShaderModule("showBox.frag.glsl");
  Marbas::ShaderStageCreateInfo showBoxVert, showBoxFrag;
  showBoxVert.shaderModule = showBoxVertModule;
  showBoxVert.stage = Marbas::ShaderType::VERTEX_SHADER;
  showBoxVert.interName = "main";
  showBoxFrag.shaderModule = showBoxFragModule;
  showBoxFrag.stage = Marbas::ShaderType::FRAGMENT_SHADER;
  showBoxFrag.interName = "main";

  auto* showBoxScreenVertModule = pipelineContext->CreateShaderModule("showBoxScreen.vert.glsl");
  auto* showBoxScreenFragModule = pipelineContext->CreateShaderModule("showBoxScreen.frag.glsl");
  Marbas::ShaderStageCreateInfo showBoxScreenVert, showBoxScreenFrag;
  showBoxScreenVert.shaderModule = showBoxScreenVertModule;
  showBoxScreenVert.stage = Marbas::ShaderType::VERTEX_SHADER;
  showBoxScreenVert.interName = "main";
  showBoxScreenFrag.shaderModule = showBoxScreenFragModule;
  showBoxScreenFrag.stage = Marbas::ShaderType::FRAGMENT_SHADER;
  showBoxScreenFrag.interName = "main";

  // create unifrom buffer

  // create pipeline and descriptorSetLayout

  std::vector<Marbas::DescriptorSetLayoutBinding> showBoxPipelinelayoutBinding;
  showBoxPipelinelayoutBinding.push_back(Marbas::DescriptorSetLayoutBinding{
      .bindingPoint = 0,
      .descriptorType = Marbas::DescriptorType::UNIFORM_BUFFER,
      .count = 1,
      .visible = Marbas::DescriptorVisible::VERTEX_SHADER,
  });
  auto* showBoxPipelineDescritorSetLayout = pipelineContext->CreateDescriptorSetLayout(showBoxPipelinelayoutBinding);

  Marbas::GraphicsPipeLineCreateInfo showBoxPipelineCreateInfo;
  showBoxPipelineCreateInfo.multisampleCreateInfo.rasterizationSamples = Marbas::SampleCount::BIT1;
  showBoxPipelineCreateInfo.shaderStageCreateInfo = {showBoxVert, showBoxFrag};
  showBoxPipelineCreateInfo.outputRenderTarget.colorAttachments = {Marbas::ColorTargetDesc{
      .isClear = true,
      .isPresent = true,  // TODO: don't present
      .sampleCount = Marbas::SampleCount::BIT1,
      .format = Marbas::ImageFormat::RGBA,
  }};
  showBoxPipelineCreateInfo.outputRenderTarget.depthAttachments = Marbas::DepthTargetDesc{
      .isClear = true,
      .sampleCount = Marbas::SampleCount::BIT1,
  };
  showBoxPipelineCreateInfo.descriptorSetLayout = showBoxPipelineDescritorSetLayout;
  auto* showBoxPipeline = pipelineContext->CreatePipeline(showBoxPipelineCreateInfo);

  // swapchain semaphores

  std::vector<Marbas::Semaphore*> aviableSemaphores(imageCount);
  std::vector<Marbas::Semaphore*> waitSemaphores(imageCount);
  for (int i = 0; i < imageCount; i++) {
    aviableSemaphores[i] = factory->CreateGPUSemaphore();
    waitSemaphores[i] = factory->CreateGPUSemaphore();
  }

  uint32_t currentFrame = 0;
  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
    auto nextImage = factory->AcquireNextImage(swapChain, aviableSemaphores[currentFrame]);

    // TODO

    factory->Present(swapChain, std::span(waitSemaphores.begin() + currentFrame, 1), nextImage);
    currentFrame = (currentFrame + 1) % imageCount;
  }
}
