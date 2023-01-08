#include <GLFW/glfw3.h>
#include <glog/logging.h>

#include "RHIFactory.hpp"
#include "imgui.h"

static void
DrawImGUI(Marbas::ImguiContext* imguiContext) {
  imguiContext->NewFrame();

  ImGui::ShowDemoWindow();
  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named
  // window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");  // Display some text (you can use a format strings too)

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f

    if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                  // when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
  }
  {
    // 3. Show another simple window.
    ImGui::Begin("Another Window");  // Pass a pointer to our bool variable (the window will have
                                     // a closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    ImGui::End();
  }

  ImGui::Render();
}

int
main(int argc, char* argv[]) {
  FLAGS_alsologtostderr = true;
  google::InitGoogleLogging(argv[0]);

  // init glfw glfwWindow  and rhiFactory
  int width = 800;
  int height = 600;
  glfwInit();
  auto factory = Marbas::RHIFactory::CreateInstance(Marbas::RendererType::VULKAN);
  auto* glfwWindow = glfwCreateWindow(width, height, "Test Window", nullptr, nullptr);

  factory->Init(glfwWindow, width, height);

  auto* imguiInterface = factory->GetImguiContext();
  auto* swapChain = factory->GetSwapchain();

  imguiInterface->SetRenderResultImage(swapChain->imageViews);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  ImFontConfig cfg;
  cfg.OversampleH = 1;
  static ImFontGlyphRangesBuilder range;
  range.Clear();
  static ImVector<ImWchar> gr;
  gr.clear();
  range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
  range.BuildRanges(&gr);

  // io.Fonts->AddFontFromFileTTF("assert/font/NotoSansSC-Light.otf", 16, &cfg, gr.Data);

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  imguiInterface->SetUpImguiBackend(glfwWindow);

  std::vector<Marbas::Semaphore*> avaliableImageSemaphore;
  std::vector<Marbas::Semaphore*> renderFinishSemaphore;

  for (int i = 0; i < swapChain->imageViews.size(); i++) {
    avaliableImageSemaphore.push_back(factory->CreateGPUSemaphore());
    renderFinishSemaphore.push_back(factory->CreateGPUSemaphore());
  }

  int currentFrame = 0;
  bool needResize = false;
  while (!glfwWindowShouldClose(glfwWindow)) {
    if (needResize) {
      glfwGetFramebufferSize(glfwWindow, &width, &height);

      if (width > 0 && height > 0) {
        swapChain = factory->RecreateSwapchain(swapChain, width, height);
        imguiInterface->SetRenderResultImage(swapChain->imageViews);
        imguiInterface->Resize(width, height);
        currentFrame = 0;
        needResize = false;
        continue;
      }
    }

    int w, h;
    glfwGetWindowSize(glfwWindow, &w, &h);
    const bool is_minimized = (w <= 0 || h <= 0);
    if (!is_minimized) {
      auto imageIndex = factory->AcquireNextImage(swapChain, avaliableImageSemaphore[currentFrame]);
      if (imageIndex == -1) {
        needResize = true;
        continue;
      }

      DrawImGUI(imguiInterface);

      imguiInterface->RenderData(imageIndex, {avaliableImageSemaphore[currentFrame], renderFinishSemaphore[0]});

      std::array finisheSemaphore = {renderFinishSemaphore[0]};
      if (factory->Present(swapChain, finisheSemaphore, imageIndex) == -1) {
        needResize = true;
        continue;
      }
    }
    currentFrame = (currentFrame + 1) % swapChain->imageViews.size();

    glfwPollEvents();
  }

  imguiInterface->ClearUp();

  for (const auto& semaphore : avaliableImageSemaphore) {
    factory->DestroyGPUSemaphore(semaphore);
  }

  for (const auto& semaphore : renderFinishSemaphore) {
    factory->DestroyGPUSemaphore(semaphore);
  }

  return 0;
}
