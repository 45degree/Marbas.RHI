---@diagnostic disable: undefined-global
target("ImguiIntegrade")
  set_kind("binary")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","sample", "ImguiIntegrade")
  set_targetdir(executedir)

  add_defines("USE_VULKAN");

  add_files("main.cc")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "glm")
  add_deps("Marbas.RHI")
target_end()
