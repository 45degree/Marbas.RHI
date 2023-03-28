---@diagnostic disable: undefined-global, undefined-field
target('imgui-docking', function()
  set_kind('static')
  set_languages('c11', 'cxx11')

  add_headerfiles('imgui/*.h')
  add_headerfiles('imgui/backends/*.h')

  add_includedirs('imgui/', { public = true })
  add_includedirs('imgui/backends/')
  add_includedirs('.', { public = true })

  add_files('imgui/*.cpp')
  add_files('imgui/backends/imgui_impl_glfw.cpp')
  add_files('imgui/backends/imgui_impl_opengl3.cpp')

  if has_config('SupportVulkan') then
    -- add_rules("UseVulkan")
    add_defines('VK_NO_PROTOTYPES')
    add_files('imgui/backends/imgui_impl_vulkan.cpp')
  end

  add_packages('glfw')
end)
