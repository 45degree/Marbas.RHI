---@diagnostic disable: undefined-field, undefined-global

package('ktx-software', function()
  set_homepage('https://github.com/KhronosGroup/KTX-Software')
  set_description('KTX (Khronos Texture) Library and Tools')

  set_urls(
    'https://github.com/KhronosGroup/KTX-Software/archive/refs/tags/$(version).zip',
    'https://github.com/KhronosGroup/KTX-Software.git'
  )

  add_configs('ktx2', { description = 'Enable ktx2 support', default = true, type = 'boolean' })
  add_configs('ktx1', { description = 'Enable ktx1 support', default = true, type = 'boolean' })
  add_configs('opengl', { description = 'Enable Opengl Texture Loader', default = true, type = 'boolean' })
  add_configs('tool', { description = 'Create KTX Tools', default = true, type = 'boolean' })
  add_configs('vulkan', { description = 'Enable Vulkan Texture Loader', default = false, type = 'boolean' })

  add_deps('cmake')

  on_load(function(package)
    if package:config('opengl') then
      package:add('deps', 'opengl')
    end

    if package:config('vulkan') then
      package:add('deps', 'vulkansdk', { option = true })
    end

    if not package:config('shared') then
      package:add('defines', 'KHRONOS_STATIC')
    end
  end)

  on_install('linux', 'windows', function(package)
    local configs = {
      '-DKTX_FEATURE_DOC=OFF',
      '-DKTX_FEATURE_JNI=OFF',
      '-DKTX_FEATURE_TESTS=OFF',
      '-DKTX_FEATURE_LOADTEST_APPS=OFF',
      '-DBUILD_TESTING=OFF',
    }

    table.insert(configs, '-DCMAKE_BUILD_TYPE=' .. (package:debug() and 'Debug' or 'Release'))
    table.insert(configs, '-DKTX_FEATURE_STATIC_LIBRARY=' .. (package:config('shared') and 'OFF' or 'ON'))

    local function add_config_arg(config_name, cmake_name)
      table.insert(configs, '-D' .. cmake_name .. '=' .. (package:config(config_name) and 'ON' or 'OFF'))
    end

    add_config_arg('ktx1', 'KTX_FEATURE_KTX1')
    add_config_arg('ktx2', 'KTX_FEATURE_KTX2')
    add_config_arg('opengl', 'KTX_FEATURE_GL_UPLAOD')
    add_config_arg('vulkan', 'KTX_FEATURE_VULKAN')
    add_config_arg('tool', 'KTX_FEATURE_TOOLS')

    import('package.tools.cmake').install(package, configs)
  end)

  on_test(function(package)
    assert(package:check_cxxsnippets({
      test = [[
      #include <cassert>
      void test() {
        ktxTexture* kTexture;
        KTX_error_code ktxresult;
        ktxresult = ktxTexture_CreateFromNamedFile("fakeFile.ktx", KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);
      }
    ]],
    }, { configs = { languages = 'c++11' }, includes = 'ktx.h' }))
  end)
end)

add_requires('ktx-software main', { configs = { shared = true, vulkan = true } })

rule('rhi-ktxloader', function()
  on_load(function(target)
    target:add('files', path.join(os.scriptdir(), 'src/*.cc'))
    target:add('includedirs', path.join(os.scriptdir(), 'include/'), { public = true })
    target:add('includedirs', path.join(os.scriptdir(), 'src/'))
    target:add('packages', 'ktx-software')
  end)
end)
