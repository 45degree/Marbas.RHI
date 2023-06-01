---@diagnostic disable: undefined-global
target('ImguiTest', function()
  set_kind('binary')
  set_languages('c11', 'cxx20')

  local executedir = path.join('$(buildir)', '$(os)', '$(arch)', '$(mode)', 'test', 'ImguiTest')
  set_targetdir(executedir)

  add_defines('USE_VULKAN')

  if is_plat('windows') then
    add_defines('NOMINMAX')
  end

  add_files('main.cc')

  add_packages('glfw', 'glog', 'shaderc', 'gtest', 'stb', 'fmt', 'glm')
  add_deps('Marbas.RHI')
end)
