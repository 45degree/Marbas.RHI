---@diagnostic disable: undefined-global
target("TestBase")
  set_kind("static")
  set_languages("c11", "cxx20")

  local executedir = path.join("$(buildir)", "$(os)", "$(arch)", "$(mode)","test")
  set_targetdir(executedir)

  add_includedirs(".", { public = true })
  add_files("**.cc")

  add_deps("Marbas.RHI")
  add_packages("tinyobjloader", "glm", "glfw")
target_end()
