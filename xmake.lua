---@diagnostic disable: undefined-global, undefined-field

add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.6")
add_requires("glog v0.5.0")
add_requires("glm 0.9.9+8")
add_requires("gtest 1.11.0")
add_requires("stb 2021.09.10")
add_requires("fmt 9.1.0")

option("SupportVulkan")
  set_default(true)
  set_category("Marbas Render Hardware Interface")
  set_description("add vulkan support")
option_end()

option("buildTest")
  set_default(false)
  set_category("Marbas Render Hardware Interface")
  set_description("build test")
option_end()

if has_config("buildTest") then
  add_requires("tinyobjloader")
end

if has_config("SupportVulkan") then
  if is_plat("linux") then
    add_requires("pkgconfig::vulkan")
    add_requires("pkgconfig::shaderc", {alias = "shaderc"})
    add_requires("spirv-cross 1.2.189+1");
  else
    option("Vulkan_SDK_Path")
      add_deps("SupportVulkan")
      set_category("Marbas Render Hardware Interface")
      set_default("D:/VulkanSDK/1.3.224.1")
      set_description("vulkan SDK dir")
      set_showmenu(true)
    option_end()
  end
end

rule("UseVulkan")
  on_load(function (target)
    if is_plat("linux") then
      target:add("packages", "pkgconfig::vulkan")
      target:add("packages", "shaderc")
    else
      local vulkanIncludePath = path.join('$(Vulkan_SDK_Path)', 'include')
      local vulkanLibPath = path.join('$(Vulkan_SDK_Path)', 'lib')
      local vulkanBinPath = path.join('$(Vulkan_SDK_Path)', 'bin')

      if not os.exists(vulkanIncludePath) then
        if os.exists(path.join('$(Vulkan_SDK_Path)', 'Include')) then
          vulkanIncludePath = path.join('$(Vulkan_SDK_Path)', 'Include')
        end
      end

      if os.exists(vulkanBinPath) then
        if os.exists(path.join('$(Vulkan_SDK_Path)', 'Bin')) then
          vulkanBinPath = path.join('$(Vulkan_SDK_Path)', 'Bin')
        end
      end

      if os.exists(vulkanLibPath) then
        if os.exists(path.join('$(Vulkan_SDK_Path)', 'Lib')) then
          vulkanLibPath = path.join('$(Vulkan_SDK_Path)', 'Lib')
        end
      end

      target:add('includedirs', vulkanIncludePath)

      for _, filepath in ipairs(os.files(vulkanLibPath .. "spriv-*.lib")) do
        target:add("links", filepath)
      end

      target:add("links", path.join(vulkanLibPath, "vulkan-1"))
      target:add('rpathdirs', vulkanBinPath)
    end
  end)
rule_end()

rule("UseDirectX12")
  on_load(function(target)
    if not is_plat("windows") then
      return
    end
    target:add('links', 'd3d12')
    target:add('links', 'dxgi')
  end)
rule_end()

target("Marbas.RHI")
  set_kind("static")
  set_languages("c11", "cxx20")

  add_includedirs("include", { public = true })
  add_includedirs("src", { public = false })
  add_files("include/**.cc")

  if is_plat("windows") then
    add_undefines("CreateSemaphore");
  end

  if has_config("SupportVulkan") then
    add_rules("UseVulkan")
    add_defines("USE_VULKAN")
    add_files("src/Vulkan/**.cc")
  end

  if has_config("use directx12") then
    add_rules("UseDirectX12")
    add_defines("USE_D3D12", 'NOMINMAX')
    add_files('src/DirectX12/**.cc')
  end

  add_deps("imgui-docking")
  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt")
target_end()

includes("test")
includes("3rdParty")
