#pragma once

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>

#include <vulkan/vulkan.hpp>

#include "ImguiContext.hpp"
#include "Vulkan/VulkanImageContext.hpp"
#include "VulkanSwapChain.hpp"

namespace Marbas {

struct VulkanImguiCreateInfo {
  GLFWwindow* glfwWindow = nullptr;
  vk::Instance instance;
  vk::PhysicalDevice physicalDevice;
  uint32_t graphicsQueueFamilyIndex = 0;
  vk::Queue graphicsQueue;
  vk::Device device;
  VulkanSwapchain* swapchain = nullptr;
};

class VulkanImguiContext final : public ImguiContext {
 public:
  explicit VulkanImguiContext(const VulkanImguiCreateInfo& createInfo);
  virtual ~VulkanImguiContext();

 public:
  void
  Resize(uint32_t width, uint32_t height) override;

  void
  ClearUp() override;

  void
  SetUpImguiBackend(GLFWwindow* windows) override;

  void
  NewFrame() override {
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void
  RenderData(uint32_t imageIndex, const ImguiRenderDataInfo& renderInfo) override;

 private:
  void
  SetUpVulkanWindowData(bool clearEnable);

  void
  CreateWindowCommandBuffer();

 private:
  uint32_t m_width = 0;
  uint32_t m_height = 0;

  GLFWwindow* m_glfwWindow = nullptr;
  const vk::Instance m_instance;
  const vk::PhysicalDevice m_physicalDevice;
  const uint32_t m_graphicsQueueFamilyIndex = 0;
  const vk::Queue m_graphicsQueue;

  vk::Device m_device;
  std::vector<vk::Framebuffer> m_framebuffers;
  std::vector<vk::Fence> m_fences;
  std::vector<vk::CommandBuffer> m_commandBuffers;
  vk::ClearValue m_clearColor;
  vk::RenderPass m_renderPass;
  vk::CommandPool m_commandPool;
  vk::DescriptorPool m_descriptorPool;

  VulkanSwapchain* m_swapchain = nullptr;
};

}  // namespace Marbas
