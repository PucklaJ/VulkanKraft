#pragma once
#include "../chunk/world.hpp"
#include <vector>

namespace physics {
class MovingObject;

class Server {
public:
  Server(const float desired_delta_time);

  void update(const chunk::World &world, const float delta_time);
  inline void add_mob(MovingObject *mob) { m_mobs.push_back(mob); }
  inline void remove_mob(const MovingObject *mob) {
    for (size_t i = 0; i < m_mobs.size(); i++) {
      if (m_mobs[i] == mob) {
        m_mobs[i] = m_mobs.back();
        m_mobs.pop_back();
      }
    }
  }

private:
  // The Server does nothing when delta_time is higher than this value
  static constexpr float max_delta_time = 0.5f;

  void _update(const chunk::World &world, const float delta_time);
  // Checks if the AABB collides with blocks of the world and corrects its
  // position if it is the case
  void _check_aabb(const chunk::World &world, MovingObject *mob) const;

  std::vector<MovingObject *> m_mobs;

  // The update call will seperated into multiple update calls if delta_time is
  // greater than this value
  const float m_desired_delta_time;
};
} // namespace physics
