---@diagnostic disable: undefined-global
target('StorageImageTest', function()
  set_kind('binary')
  set_languages('c11', 'cxx20')

  local executedir = path.join('$(buildir)', '$(os)', '$(arch)', '$(mode)', 'test', 'StorageImageTest')
  set_targetdir(executedir)

  add_defines('USE_VULKAN')
  add_rules('utils.glsl2spv', { outputdir = executedir })

  add_files('main.cc')
  add_files('shader.frag')
  add_files('shader.vert')

  add_packages('glfw', 'glog', 'shaderc', 'gtest', 'stb', 'fmt', 'glm')
  add_deps('Marbas.RHI', 'TestBase')
end)
