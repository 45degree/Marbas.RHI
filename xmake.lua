---@diagnostic disable: undefined-global, undefined-field

add_rules('mode.debug', 'mode.release')

add_requires('glfw 3.3.6')
add_requires('glog v0.5.0')
add_requires('glm 0.9.9+8')
add_requires('gtest 1.11.0')
add_requires('stb 2021.09.10')
add_requires('fmt 9.1.0')
add_requires('spirv-cross c77b09b57c27837dc2d41aa371ed3d236ce9ce47')
add_requires('abseil 20230125.2')

option('SupportVulkan', function()
  set_default(true)
  set_category('Marbas Render Hardware Interface')
  set_description('add vulkan support')
end)

option('buildTest', function()
  set_default(false)
  set_category('Marbas Render Hardware Interface')
  set_description('build test')
end)

if has_config('buildTest') then
  add_requires('tinyobjloader')
end

if has_config('SupportVulkan') then
  add_requires('vulkan-hpp v1.3.243', { verify = false })
  add_requires('vulkan-headers 1.3.243+0', { verify = false })
  add_requires('shaderc v2022.2')
end

target('Marbas.RHI', function()
  set_kind('static')
  set_languages('c11', 'cxx20')

  add_includedirs('include', { public = true })
  add_includedirs('src')
  add_files('include/**.cc')

  if is_plat('windows') then
    add_undefines('CreateSemaphore')
    add_defines('NOMINMAX')
  end

  if has_config('SupportVulkan') then
    add_defines('USE_VULKAN')
    add_defines('VK_NO_PROTOTYPES')
    add_files('src/Vulkan/**.cc')
    add_packages('vulkan-hpp')
  end

  add_deps('imgui-docking')
  add_packages('glfw', 'glog', 'shaderc', 'gtest', 'stb', 'fmt', 'spirv-cross', 'abseil')
end)

includes('test')
includes('3rdParty')
