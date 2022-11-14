---@diagnostic disable: undefined-global, undefined-field
target('imgui-docking')
  set_kind("static")
  set_languages("c11", "cxx11")

  add_headerfiles("imgui/*.h")
  add_headerfiles("imgui/backends/*.h")

  add_includedirs("imgui/")
  add_includedirs("imgui/backends/")

  add_files("imgui/*.cpp")
  add_files("imgui/backends/imgui_impl_glfw.cpp")
  add_files("imgui/backends/imgui_impl_opengl3.cpp")

  if has_config("use vulkan") then
    add_files("imgui/backends/imgui_impl_vulkan.cpp")
  end

  add_packages("glfw")
target_end()
