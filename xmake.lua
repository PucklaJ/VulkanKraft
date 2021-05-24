set_project("VulkanKraft")
add_requires("glfw")

add_rules("mode.debug", "mode.release")
target("vulkankraft")
  set_kind("binary")
  set_languages("cxx17")
  add_packages("glfw")

  add_files("src/*.cpp", "src/core/*.cpp")
  add_headerfiles("src/*.hpp", "src/core/*.hpp")