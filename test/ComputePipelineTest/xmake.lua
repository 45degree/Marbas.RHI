---@diagnostic disable: undefined-global

target("ComputePipelineTest")
  set_kind("binary")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","ComputePipelineTest")

  set_targetdir(executedir)
  add_rules("utils.glsl2spv", {outputdir = executedir})

  add_defines("USE_VULKAN");

  add_includedirs(".", { public = true })
  add_files("**.cc")
  add_files("**.glsl")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "glm")
  add_deps("Marbas.RHI", "TestBase")
target_end()
