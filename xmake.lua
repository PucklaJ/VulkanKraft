set_project("VulkanKraft")
add_requires("glfw", "glm", "stb", "vulkan-hpp")

rule("shader")
    set_extensions(".vert", ".frag", ".glsl")
    on_buildcmd_file(function(target, batchcmds, sourcefile_glsl, opt)
            import("lib.detect.find_tool")
            local glslc = assert(find_tool("glslc"), "glslc not found!")
            local hpp_gen = path.join("build", target:plat(), target:arch(), "release", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
            assert(os.exists(hpp_gen), "hpp_gen has not been built yet!")

            local flags = {}
            local shader_stage
            if path.extension(sourcefile_glsl) == ".glsl" then
                local base = path.basename(sourcefile_glsl)
                if base == "vert" then
                    shader_stage = "vert"
                    table.insert(flags, "-fshader-stage=vertex")
                elseif base == "frag" then
                    shader_stage = "frag"
                    table.insert(flags, "-fshader-stage=fragment")
                else
                    assert(false, "cannot determine shader stage of " .. sourcefile_glsl)
                end
            elseif path.extension(sourcefile_glsl) == ".vert" then
              shader_stage = "vert"
            elseif path.extension(sourcefile_glsl) == ".frag" then
              shader_stage = "frag"
            end
            local spv_file = path.join("shaders_spv", path.basename(sourcefile_glsl) .. "." .. shader_stage .. ".spv")
            table.insert(flags, sourcefile_glsl)
            table.insert(flags, "-o")
            table.insert(flags, spv_file)

            batchcmds:show_progress(opt.progress, "${color.build.object}compiling.glsl %s", sourcefile_glsl)
            batchcmds:mkdir("shaders_spv")
            batchcmds:vrunv(glslc.program, flags)
            batchcmds:vrunv(hpp_gen, {spv_file, path.join(os.scriptdir(), "resources/shaders"), "true"})

            batchcmds:add_depfiles(sourcefile_glsl, hpp_gen)
    end)

rule("texture")
  set_extensions(".png")
  on_buildcmd_file(function(target, batchcmds, sourcefile_png, opt)
    local hpp_gen = path.join("build", target:plat(), target:arch(), "release", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
    assert(os.exists(hpp_gen), "hpp_gen has not been built yet!")

    batchcmds:show_progress(opt.progress, "${color.build.object}generating.hpp %s", sourcefile_png)
    batchcmds:vrunv(hpp_gen, {sourcefile_png, path.join(os.scriptdir(), "resources/textures")})

    batchcmds:add_depfiles(sourcefile_png, hpp_gen)
  end)

rule("font")
  set_extensions(".otf", ".ttf")
  on_buildcmd_file(function(target, batchcmds, sourcefile_ttf, opt)
    local hpp_gen = path.join("build", target:plat(), target:arch(), "release", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
    assert(os.exists(hpp_gen), "hpp_gen has not been built yet!")

    batchcmds:show_progress(opt.progress, "${color.build.object}generating.hpp %s", sourcefile_ttf)
    batchcmds:vrunv(hpp_gen, {sourcefile_ttf, path.join(os.scriptdir(), "resources/fonts")})

    batchcmds:add_depfiles(sourcefile_ttf, hpp_gen)
  end)
rule_end()



target("hpp_gen")
  set_kind("binary")
  set_languages("cxx17")
  add_rules("mode.release")
  add_files("cmd/hpp_gen/main.cpp")
target_end()

add_rules("mode.debug", "mode.release")
target("vulkankraft")
  set_kind("binary")
  set_languages("cxx17")
  add_deps("hpp_gen")
  add_packages("glfw", "glm", "stb", "vulkan-hpp")
  if is_plat("windows") then
    add_syslinks("C:\\VulkanSDK\\1.2.135.0\\Lib" .. (is_arch("x86") and "32" or "") .. "\\vulkan-1")
  else
    add_syslinks("vulkan", "pthread")
  end
  add_includedirs("resources")
  add_rules("shader", "texture", "font")

  add_files("src/*.cpp", 
            "src/core/*.cpp",
            "src/core/vulkan/*.cpp",
            "src/core/text/*.cpp",
            "src/chunk/*.cpp",
            "src/block/*.cpp")
  add_headerfiles("src/*.hpp",
                  "src/core/*.hpp",
                  "src/core/vulkan/*.hpp",
                  "src/core/text/*.hpp",
                  "src/chunk/*.hpp",
                  "src/block/*.hpp")
  add_files("shaders/*",
            "textures/*",
            "fonts/*")