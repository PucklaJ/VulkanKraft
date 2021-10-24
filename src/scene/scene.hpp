#pragma once
#include "../core/vulkan/render_call.hpp"
#include "../core/window.hpp"
#include <memory>

namespace scene {
class Scene {
public:
  virtual ~Scene() {}
  // Gets called every frame before render
  // delta_time .... The time between the last frame and this in seconds
  // If a Scene is returned the scene gets switched to that one
  virtual std::unique_ptr<Scene> update(core::Window &window,
                                        const float delta_time) = 0;
  // Gets called every frame after update
  // delta_time .... The time between the last frame and this in seconds
  virtual void render(const core::vulkan::RenderCall &render_call,
                      const float delta_time) = 0;
};
} // namespace scene
