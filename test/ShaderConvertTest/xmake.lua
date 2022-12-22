
---@diagnostic disable: undefined-global
target("ShaderConvertTest")
  set_kind("binary")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","test", "ShaderConvertTest")
  set_targetdir(executedir)

  add_defines("USE_VULKAN");
  add_defines("USE_D3D12", 'NOMINMAX')

  add_rules("utils.glsl2spv", {outputdir = executedir})

  add_files("main.cc")
  add_files("shader.vert.glsl")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "glm")
  add_deps("Marbas.RHI", "TestBase")
target_end()
