---@diagnostic disable: undefined-global, undefined-field

add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.6")
add_requires("glog v0.5.0")
add_requires("glm 0.9.9+8")
add_requires("gtest 1.11.0")
add_requires("stb 2021.09.10")
add_requires("fmt 9.1.0")
add_requires("spirv-cross c77b09b57c27837dc2d41aa371ed3d236ce9ce47")

option("SupportVulkan")
  set_default(true)
  set_category("Marbas Render Hardware Interface")
  set_description("add vulkan support")
option_end()

if is_host("windows") then
  option("Vulkan_SDK_Path")
    add_deps("SupportVulkan")
    set_category("Marbas Render Hardware Interface")
    set_default("$(env VK_SDK_PATH)")
    set_description("vulkan SDK dir")
    set_showmenu(true)
  option_end()
end

option("buildTest")
  set_default(false)
  set_category("Marbas Render Hardware Interface")
  set_description("build test")
option_end()

if has_config("buildTest") then
  add_requires("tinyobjloader")
end

if has_config("SupportVulkan") then
  if is_host("linux") then
    add_requires("pkgconfig::vulkan")
    add_requires("pkgconfig::shaderc", {alias = "shaderc"})
  else
    add_requires("shaderc v2022.2")
  end
end

rule("UseVulkan")
  on_load(function (target)
    if is_host("linux") then
      target:add("packages", "pkgconfig::vulkan")
      target:add("packages", "shaderc")
    else
      local vulkanIncludePath = path.join('$(Vulkan_SDK_Path)', 'include')
      local vulkanLibPath = path.join('$(Vulkan_SDK_Path)', 'lib')

      -- copy the vulkan header and libary
      local tmpVulkanDir = "$(buildir)/vulkan";
      local tmpVulkanIncludeDir = path.join(tmpVulkanDir, "include/")
      local tmpVukanLibDir = path.join(tmpVulkanDir, "lib")
      if os.exists(tmpVulkanDir) then
        os.rm(tmpVulkanDir);
      end
      os.mkdir(tmpVulkanDir);
      os.mkdir(tmpVulkanIncludeDir);
      os.mkdir(tmpVukanLibDir);

      os.cp(path.join(vulkanIncludePath, "vulkan"), path.join(tmpVulkanIncludeDir, "vulkan")) -- copy header
      os.cp(path.join(vulkanLibPath, "vulkan-1.*"), tmpVukanLibDir) -- copy lib

      target:add('includedirs', tmpVulkanIncludeDir)
      target:add("linkdirs", tmpVukanLibDir)
      target:add("links", "vulkan-1")
    end
  end)
rule_end()

target("Marbas.RHI")
  set_kind("static")
  set_languages("c11", "cxx20")

  add_includedirs("include", { public = true })
  add_includedirs("src")
  add_files("include/**.cc")

  if is_plat("windows") then
    add_undefines("CreateSemaphore");
    add_defines("NOMINMAX")
  end

  if has_config("SupportVulkan") then
    add_rules("UseVulkan")
    add_defines("USE_VULKAN")
    add_files("src/Vulkan/**.cc")
  end

  -- add_rules("rhi-ktxloader")

  add_deps("imgui-docking")
  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "spirv-cross")
target_end()

includes("test")
includes("3rdParty")
-- includes("extensions/ktx")
