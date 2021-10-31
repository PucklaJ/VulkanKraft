#pragma once

#include "block/server.hpp"
#include "chunk/world.hpp"
#include "core/resource_hodler.hpp"
#include "core/settings.hpp"
#include "core/text/text.hpp"
#include "physics/server.hpp"
#include "player.hpp"
#include "scene/scene.hpp"

class InGameScene : public scene::Scene {
public:
  InGameScene(const core::vulkan::Context &context,
              core::ResourceHodler &resource_hodler,
              const core::Settings &settings, const glm::mat4 &projection,
              const bool new_world = false, const size_t world_seed = 0);
  ~InGameScene();

  std::unique_ptr<scene::Scene> update(core::Window &window,
                                       const float delta_time) override;
  void render(const core::vulkan::RenderCall &render_call,
              const float delta_time) override;

private:
  static constexpr int reseed_keyboard_button = GLFW_KEY_F8;
  static constexpr int place_player_keyboard_button = GLFW_KEY_F9;

  block::Server m_block_server;
  physics::Server m_physics_server;

  core::text::Text m_fps_text;
  core::text::Text m_position_text;
  core::text::Text m_look_text;
  core::text::Text m_vel_text;

  Player m_player;
  chunk::World m_world;

  core::Shader &m_chunk_shader;

  float m_fog_max_distance;
  chunk::GlobalUniform m_chunk_global;
  const glm::mat4 &m_projection;
};