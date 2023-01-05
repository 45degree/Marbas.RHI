---@diagnostic disable: undefined-global
target("DepthBufferTest")
  set_kind("binary")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","test", "DepthBufferTest")
  set_targetdir(executedir)

  add_defines("USE_VULKAN");
  add_rules("utils.glsl2spv", {outputdir = executedir})

  after_build(function ()
    os.cp('$(scriptdir)/texture.jpg', executedir)
  end)

  add_files("main.cc")
  add_files("shader.frag")
  add_files("shader.vert")

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "glm")
  add_deps("Marbas.RHI", "TestBase")
target_end()
