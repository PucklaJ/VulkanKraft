#include "server.hpp"
#include "moving_object.hpp"

namespace physics {

Server::Server(const float desired_delta_time)
    : m_desired_delta_time(desired_delta_time) {}

void Server::update(const chunk::World &world, const float delta_time) {
  // TODO: handle too long delta time
  if (delta_time > max_delta_time) {
    return;
  }

  _update(world, delta_time);
}

void Server::_update(const chunk::World &world, const float delta_time) {
  for (auto *mob : m_mobs) {
    mob->_compute_new_aabb_position(delta_time);
    _check_aabb(world, mob);
    mob->_compute_new_position();
  }
}

void Server::_check_aabb(const chunk::World &world, MovingObject *mob) const {
  // Get the chunks in which the AABB resides
  const auto chunk_pos(chunk::World::get_chunk_position(mob->m_aabb.position));
  if (world.m_chunks.find(chunk_pos) == world.m_chunks.end()) {
    // There is no chunk at the position of the AABB
    return;
  }

  std::vector<std::shared_ptr<chunk::Chunk>> colliding_chunks;
  colliding_chunks.reserve(9);

  // Get the chunk and all its eight neighbors
  const auto chunk(world.m_chunks.at(chunk_pos));
  if (auto chunk_aabb(chunk->to_physics_aabb());
      chunk_aabb.collide(mob->m_aabb)) {
    colliding_chunks.push_back(chunk);
  }

  if (auto left(chunk->get_left()); left) {
    if (auto chunk_aabb(left->to_physics_aabb());
        chunk_aabb.collide(mob->m_aabb)) {
      colliding_chunks.push_back(left);
    }
  }
  if (auto right(chunk->get_right()); right) {
    if (auto chunk_aabb(right->to_physics_aabb());
        chunk_aabb.collide(mob->m_aabb)) {
      colliding_chunks.push_back(right);
    }
  }
  if (auto front(chunk->get_front()); front) {
    if (auto chunk_aabb(front->to_physics_aabb());
        chunk_aabb.collide(mob->m_aabb)) {
      colliding_chunks.push_back(front);
    }
    if (auto front_left(front->get_left()); front_left) {
      if (auto chunk_aabb(front_left->to_physics_aabb());
          chunk_aabb.collide(mob->m_aabb)) {
        colliding_chunks.push_back(front_left);
      }
    }
    if (auto front_right(front->get_right()); front_right) {
      if (auto chunk_aabb(front_right->to_physics_aabb());
          chunk_aabb.collide(mob->m_aabb)) {
        colliding_chunks.push_back(front_right);
      }
    }
  }
  if (auto back(chunk->get_back()); back) {
    if (auto chunk_aabb(back->to_physics_aabb());
        chunk_aabb.collide(mob->m_aabb)) {
      colliding_chunks.push_back(back);
    }
    if (auto back_left(back->get_left()); back_left) {
      if (auto chunk_aabb(back_left->to_physics_aabb());
          chunk_aabb.collide(mob->m_aabb)) {
        colliding_chunks.push_back(back_left);
      }
    }
    if (auto back_right(back->get_right()); back_right) {
      if (auto chunk_aabb(back_right->to_physics_aabb());
          chunk_aabb.collide(mob->m_aabb)) {
        colliding_chunks.push_back(back_right);
      }
    }
  }

  // The AABB doesn't collide with any of the chunks (doesn't make sense but
  // just handle it anyways)
  if (colliding_chunks.empty()) {
    return;
  }

  // Stores how much to push the AABB
  glm::vec3 push(0.0f, 0.0f, 0.0f);

  // Loop over all colliding chunks
  for (const auto cc : colliding_chunks) {
    // Loop over all blocks
    for (size_t x = 0; x < chunk::block_width; x++) {
      for (size_t y = 0; y < chunk::block_height; y++) {
        for (size_t z = 0; z < chunk::block_depth; z++) {
          const auto &block{cc->get_block(x, y, z)};
          if (block::Server::block_is_solid(block.type)) {
            // Push the AABB if it collides with the block
            const auto block_aabb(block.to_physics_aabb(
                glm::vec3(static_cast<float>(x + cc->get_position().x),
                          static_cast<float>(y),
                          static_cast<float>(z + cc->get_position().y))));
            if (block_aabb.collide(mob->m_aabb, push.x, push.y, push.z)) {
              // We can't push the mob->m_aabb when there is another block in
              // the way
              push.x = !(push.x < 0.0f && !block.left_face() ||
                         push.x > 0.0f && !block.right_face()) *
                       push.x;

              push.y = !(push.y < 0.0f && !block.bot_face() ||
                         push.y > 0.0f && !block.top_face()) *
                       push.y;

              push.z = !(push.z < 0.0f && !block.back_face() ||
                         push.z > 0.0f && !block.front_face()) *
                       push.z;

              const glm::vec3 push_abs(core::math::abs(push.x),
                                       core::math::abs(push.y),
                                       core::math::abs(push.z));

              // Determine the smallest push value
              const auto index{(push.y != 0.0f &&
                                (push.x == 0.0f || push_abs.y <= push_abs.x) &&
                                (push.z == 0.0f || push_abs.y <= push_abs.z)) *
                                   1 +
                               (push.z != 0.0f &&
                                (push.x == 0.0f || push_abs.z < push_abs.x) &&
                                (push.y == 0.0f || push_abs.z < push_abs.y)) *
                                   2};
#ifndef NDEBUG
              assert(index == 0 || index == 1 || index == 2);
#endif
              mob->m_aabb.position[index] += push[index];
              mob->velocity[index] =
                  (push[index] == 0.0f) * mob->velocity[index];
            }
          }
        }
      }
    }
  }
}
} // namespace physics
