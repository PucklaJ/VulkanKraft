#include "core/exception.hpp"
#include "core/fps_timer.hpp"
#include "core/log.hpp"
#include "core/settings.hpp"
#include "core/vulkan/context.hpp"
#include "core/window.hpp"

int main(int args, char *argv[]) {
  core::Settings settings;
  core::FPSTimer timer(settings);

  try {
    core::Window window(settings.window_width, settings.window_height,
                        core::Settings::window_title);
    core::vulkan::Context context(window, settings);

    while (!window.should_close()) {
      auto delta_timer(timer.begin_frame());
      window.poll_events();

      if (const auto render_call(context.render_begin()); render_call) {
      }
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
  }

  return 0;
}