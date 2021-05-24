set_project("VulkanKraft")

add_rules("mode.debug", "mode.release")
target("vulkankraft")
  set_kind("binary")
  set_languages("cxx17")

  add_files("src/*.cpp")
  add_headerfiles("src/*.hpp")