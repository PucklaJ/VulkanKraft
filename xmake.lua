set_project("VulkanKraft")
add_requires("glfw", "glm", "stb", "vulkan-hpp")

rule("shader")
    set_extensions(".vert", ".frag", ".glsl")
    on_buildcmd_file(function(target, batchcmds, sourcefile_glsl, opt)
            import("lib.detect.find_tool")
            local glslc = assert(find_tool("glslc"), "glslc not found!")
            local hpp_gen = path.join("build", target:plat(), target:arch(), "release", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
            if not os.exists(hpp_gen) then 
                hpp_gen = path.join("build", target:plat(), target:arch(), "debug", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
                assert(os.exists(hpp_gen), "hpp_gen has not been built yet. Run \"xmake build hpp_gen\"")
            end

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
    if not os.exists(hpp_gen) then 
        hpp_gen = path.join("build", target:plat(), target:arch(), "debug", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
        assert(os.exists(hpp_gen), "hpp_gen has not been built yet. Run \"xmake build hpp_gen\"")
    end

    batchcmds:show_progress(opt.progress, "${color.build.object}generating.hpp %s", sourcefile_png)
    batchcmds:vrunv(hpp_gen, {sourcefile_png, path.join(os.scriptdir(), "resources/textures")})

    batchcmds:add_depfiles(sourcefile_png, hpp_gen)
  end)

rule("font")
  set_extensions(".otf", ".ttf")
  on_buildcmd_file(function(target, batchcmds, sourcefile_ttf, opt)
    local hpp_gen = path.join("build", target:plat(), target:arch(), "release", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
    if not os.exists(hpp_gen) then 
        hpp_gen = path.join("build", target:plat(), target:arch(), "debug", "hpp_gen" .. (target:is_plat("windows") and ".exe" or ""))
        assert(os.exists(hpp_gen), "hpp_gen has not been built yet. Run \"xmake build hpp_gen\"")
    end

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

target("resources")
  set_kind("static")

  on_load(function (target)
    -- xmake fails on startup when this folder is missing
    os.mkdir("resources")
  end)

  add_deps("hpp_gen")
  add_rules("shader", "texture", "font")
  add_files("shaders/*",
            "textures/*.png",
            "fonts/*")
target_end()

add_rules("mode.debug", "mode.release")
target("core")
  set_kind("static")
  set_languages("cxx17")
  add_deps("resources")
  add_packages("glfw", "glm", "stb", "vulkan-hpp")
  if is_plat("windows") then
    local vulkan_path = os.getenv("VK_SDK_PATH")
    if vulkan_path == nil then
        print("couldn't find an installation of the Vulkan SDK. Make sure that the VK_SDK_PATH environment variable is set")
    else
        add_syslinks(path.join(vulkan_path, "Lib" .. (is_arch("x86") and "32" or ""), "vulkan-1"))
    end
  else
    add_syslinks("vulkan", "pthread")
  end
  add_includedirs("resources")

  add_files("src/core/*.cpp",
            "src/core/vulkan/*.cpp",
            "src/core/text/*.cpp")
  add_headerfiles("src/core/*.hpp",
                  "src/core/vulkan/*.hpp",
                  "src/core/text/*.hpp")

target("vulkankraft")
  set_kind("binary")
  set_languages("cxx17")
  add_deps("core")
  add_packages("glfw", "glm", "stb", "vulkan-hpp")

  add_files("src/*.cpp",
            "src/chunk/*.cpp",
            "src/block/*.cpp",
            "src/world_gen/*.cpp",
            "src/physics/*.cpp")
  add_headerfiles("src/*.hpp",
                  "src/chunk/*.hpp",
                  "src/block/*.hpp",
                  "src/world_gen/*.hpp",
                  "src/physics/*.hpp")

target("perlin_noise_test")
  set_enabled(is_mode("debug"))
  set_kind("binary")
  set_languages("cxx17")
  add_deps("core")
  add_packages("glm")

  add_files("src/world_gen/perlin_noise.cpp",
            "src/world_gen/perlin_noise_test/*.cpp")
  add_headerfiles("src/world_gen/perlin_noise.hpp")
  add_includedirs("resources")

target("texture_2d_test")
  set_enabled(is_mode("debug"))
  set_kind("binary")
  set_languages("cxx17")
  add_deps("core")
  add_packages("glm", "stb")

  add_files("src/core/texture_2d_test/main.cpp")

target("physics_test")
  set_enabled(is_mode("debug"))
  set_kind("binary")
  set_languages("cxx17")
  add_packages("glm")

  add_files("src/physics/aabb.cpp",
            "src/physics/physics_test/main.cpp")
  add_headerfiles("src/physics/aabb.hpp")