#include "core/exception.hpp"
#include "core/log.hpp"
#include "core/window.hpp"
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

constexpr uint32_t window_width = 840;
constexpr uint32_t window_height = 480;
constexpr char window_title[] = "VulkanKraft";

int main(int args, char *argv[]) {
  try {
    core::Window window(window_width, window_height, window_title);

    while (!window.should_close()) {
      window.poll_events();
      std::this_thread::sleep_for(10ms);
    }
  } catch (const core::VulkanKraftException &e) {
    core::Log::error(e.what());
    return 1;
  }

  return 0;
}