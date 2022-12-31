---@diagnostic disable: undefined-global
target("MultiPassTest")
  set_kind("binary")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","test", "MultiPassTest")
  set_targetdir(executedir)

  add_defines("USE_VULKAN");
  add_rules("utils.glsl2spv", {outputdir = executedir, targetenv = "opengl"})

  add_files("*.cc")
  add_files("**.glsl")

  after_build(function ()
    os.cp('$(scriptdir)/*.jpg', executedir)
    os.cp('$(scriptdir)/*.png', executedir)
  end)

  add_packages("glfw", "glog", "shaderc", "gtest", "stb", "fmt", "glm")
  add_deps("Marbas.RHI", "TestBase")
target_end()
