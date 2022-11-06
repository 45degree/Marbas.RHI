---@diagnostic disable: undefined-global
target("HelloTriangle")
  set_kind("binary")
  set_languages("c11", "cxx20")

  add_defines("USE_VULKAN");

  add_files("main.cc")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt")
  add_deps("Marbas.RHI")
target_end()
