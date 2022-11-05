---@diagnostic disable: undefined-global

add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.6")
add_requires("glog v0.5.0")
add_requires("glm 0.9.9+8")
add_requires("gtest 1.11.0")
add_requires("shaderc")
add_requires("stb 2021.09.10")

if is_plat("linux") then
  add_requires("pkgconfig::vulkan")
  add_requires("spirv-cross 1.2.189+1");
else
  option("Vulkan SDK Path")
    set_default("D:/VulkanSDK/1.3.224.1")
    set_description("Vulkan SDK Path")
    set_showmenu(true)
  option_end()
end

rule("LoadVulkan")
  on_load(function (target)
    if is_plat("linux") then
      target:add("packages", "pkgconfig::vulkan")
      target:add("packages", "shaderc")
    else
      local vulkanIncludePath = path.join('$(Vulkan SDK Path)', 'include')
      local vulkanLibPath = path.join('$(Vulkan SDK Path)', 'lib')
      local vulkanBinPath = path.join('$(Vulkan SDK Path)', 'bin')

      if not os.exists(vulkanIncludePath) then
        if os.exists(path.join('$(Vulkan SDK Path)', 'Include')) then
          vulkanIncludePath = path.join('$(Vulkan SDK Path)', 'Include')
        end
      end

      if os.exists(vulkanBinPath) then
        if os.exists(path.join('$(Vulkan SDK Path)', 'Bin')) then
          vulkanBinPath = path.join('$(Vulkan SDK Path)', 'Bin')
        end
      end

      if os.exists(vulkanLibPath) then
        if os.exists(path.join('$(Vulkan SDK Path)', 'Lib')) then
          vulkanLibPath = path.join('$(Vulkan SDK Path)', 'Lib')
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

target("Marbas.RHI")
  set_kind("static")
  set_languages("c11", "cxx20")

  add_defines("USE_VULKAN");

  add_includedirs("include", { public = true })
  add_includedirs("src", { public = false })
  add_files("src/**.cc")

  add_rules("LoadVulkan")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb")
target_end()
