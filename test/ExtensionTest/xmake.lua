target("ExtensionTest")
  set_kind("binary")
  set_languages("c11", "cxx20")

  add_files("main.cc")
  add_deps("Marbas.RHI")
  add_packages("glfw")
target_end()
