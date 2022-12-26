---@diagnostic disable: undefined-global, undefined-field
target('imgui-docking')
  set_kind("static")
  set_languages("c11", "cxx11")

  add_headerfiles("imgui/*.h")
  add_headerfiles("imgui/backends/*.h")

  add_includedirs("imgui/", {public = true})
  add_includedirs("imgui/backends/")
  add_includedirs(".", { public = true })

  add_files("imgui/*.cpp")
  add_files("imgui/backends/imgui_impl_glfw.cpp")
  add_files("imgui/backends/imgui_impl_opengl3.cpp")


  if has_config("SupportVulkan") then
    add_rules("UseVulkan")
    add_files("imgui/backends/imgui_impl_vulkan.cpp")
  end

  add_packages("glfw")
target_end()

package("directxShaderCompiler")
  set_homepage("https://github.com/microsoft/DirectXShaderCompiler/")
  set_description("DirectX Shader Compiler")
  set_license("LLVM")

  local date = {
    ["1.7.2212"] = "2022_12_16",
  }

  if is_plat("windows") then
    add_urls("https://github.com/microsoft/DirectXShaderCompiler/releases/download/v$(version)", {
      version = function (version)
        return version .. "/dxc_" .. date[tostring(version)]..".zip"
      end
    })
    add_versions("1.7.2212", "ed77c7775fcf1e117bec8b5bb4de6735af101b733d3920dda083496dceef130f")
  else
    add_deps("cmake", "python 3.x", "ninja")
    add_urls("https://github.com/microsoft/DirectXShaderCompiler/archive/refs/tags/v$(version).zip")
    add_versions("1.7.2212", "3d86b2d3142738d86d00af52d7a5ed98b503dbf043a1d95e52ea1cc63047d711")
  end

  on_install("windows|x64", function (package)
    os.cp("bin/x64/*", package:installdir("bin"))
    os.cp("inc/*", package:installdir("include"))
    os.cp("lib/x64/*", package:installdir("lib"))
    package:addenv("PATH", "bin")
  end)

  on_install("windows|x86", function (package)
    os.cp("bin/x86/*", package:installdir("bin"))
    os.cp("inc/*", package:installdir("include"))
    os.cp("lib/x86/*", package:installdir("lib"))
    package:addenv("PATH", "bin")
  end)

  on_install("linux", function(package)
    local configs = {}
    table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
    table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
    table.insert(configs, "-GNinja")
    table.insert(configs, "-C ../cmake/caches/PredefinedParams.cmake")
    import("package.tools.cmake").install(package, configs)
  end)

  on_test(function (package)
      os.vrun("dxc -help")
      assert(package:has_cxxfuncs("DxcCreateInstance", {includes = {"dxcapi.h"}}))
  end)
