---@diagnostic disable: undefined-global

add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.6")
add_requires("glog v0.5.0")
add_requires("glm 0.9.9+8")
add_requires("gtest 1.11.0")
add_requires("shaderc")
add_requires("stb 2021.09.10")


target("Marbas.RHI")
  set_kind("static")
  set_languages("c11", "cxx20")

  add_defines("USE_VULKAN");

  add_includedirs("include", { public = true })
  add_includedirs("src", { public = false })
  add_files("src/**.cc")
target_end()
