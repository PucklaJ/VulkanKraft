#include "world.hpp"
#include "../core/exception.hpp"
#include "../core/fps_timer.hpp"
#include "../core/log.hpp"
#include <chrono>
#include <limits>
#include <set>
#include <sstream>

namespace chunk {
World::World(const ::core::vulkan::Context &context,
             const block::Server &block_server)
    : m_context(context), m_block_server(block_server) {

  world_save_folder =
      std::filesystem::temp_directory_path() / "vulkankraft_world";

  m_save_world = std::make_unique<save::World>(world_save_folder);
  const auto save_meta_data(m_save_world->read_meta_data());
  if (save_meta_data) {
    m_world_generation.seed(save_meta_data->seed);
  } else {
    save::World::MetaData meta_data{static_cast<size_t>(time(nullptr))};
    m_save_world->write_meta_data(meta_data);
    m_world_generation.seed(meta_data.seed);
  }
}

World::~World() {
  m_running = false;
  if (m_chunk_update_thread)
    m_chunk_update_thread->join();

  // Store all chunks
  for (const auto &[chunk_pos, chunk] : m_chunks) {
    m_save_world->store_chunk(chunk_pos, chunk->to_stored_blocks());
  }
}

void World::place_block(const glm::ivec3 &position, const block::Type block) {
  std::lock_guard lk(m_chunks_mutex);

  {
    std::stringstream stream;
    stream << "Placing block " << block::type_as_str(block) << " at ";
    stream << "(" << position.x << "; " << position.y << "; " << position.z
           << ")";
    core::Log::debug(stream.str());
  }

  if (position.y >= block_height) {
    throw core::VulkanKraftException(std::to_string(position.y) +
                                     " is above height limit");
  }

  const auto chunk_pos(get_chunk_position(position));

  auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    (*chunk)->set(chunk_block_position.x, chunk_block_position.y,
                  chunk_block_position.z, block);
    (*chunk)->generate_block_change(m_block_server, chunk_block_position);
    return;
  }

  std::stringstream stream;
  stream << "attempted to place block " << std::boolalpha << block
         << " at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

void World::destroy_block(const glm::ivec3 &position) {
  std::lock_guard lk(m_chunks_mutex);

  {
    std::stringstream stream;
    stream << "Destroying block at ";
    stream << "(" << position.x << "; " << position.y << "; " << position.z
           << ")";
    ::core::Log::debug(stream.str());
  }

  const auto chunk_pos(get_chunk_position(position));

  auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    (*chunk)->set(chunk_block_position.x, chunk_block_position.y,
                  chunk_block_position.z, block::Type::AIR);
    (*chunk)->generate_block_change(m_block_server, chunk_block_position);
    return;
  }

  std::stringstream stream;
  stream << "attempted to destroy block at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

block::Type World::show_block(const glm::ivec3 &position) {
  std::lock_guard lk(m_chunks_mutex);

  const auto chunk_pos(get_chunk_position(position));

  const auto chunk(_get_chunk(chunk_pos));
  if (chunk) {
    const glm::ivec3 chunk_block_position(
        position.x - (*chunk)->get_position().x, position.y,
        position.z - (*chunk)->get_position().y);

    return (*chunk)->get(chunk_block_position.x, chunk_block_position.y,
                         chunk_block_position.z);
  }

  std::stringstream stream;
  stream << "attempted to show block at position ";
  stream << "(" << position.x << "; " << position.y << "; " << position.z
         << ")";

  throw ::core::VulkanKraftException(stream.str());
}

std::optional<glm::ivec3> World::raycast_block(const physics::Ray &ray,
                                               physics::Ray::Face &face,
                                               float &distance) {
  std::lock_guard lk(m_chunks_mutex);

  // Get chunk of ray
  const auto chunk_pos(get_chunk_position(ray.origin));

  const auto _ray_chunk = _get_chunk(chunk_pos);
  if (!_ray_chunk) {
    return std::nullopt;
  }
  const auto ray_chunk = *_ray_chunk;

  // Raycast ray chunk and neighbouring
  std::set<std::shared_ptr<Chunk>> ray_chunks = {
      ray_chunk, ray_chunk->get_front(), ray_chunk->get_back(),
      ray_chunk->get_left(), ray_chunk->get_right()};
  if (auto left = ray_chunk->get_left(); left) {
    ray_chunks.emplace(left->get_front());
    ray_chunks.emplace(left->get_back());
  }
  if (auto right = ray_chunk->get_right(); right) {
    ray_chunks.emplace(right->get_front());
    ray_chunks.emplace(right->get_back());
  }

  distance = std::numeric_limits<float>::max();
  glm::vec3 chunk_world_pos;
  glm::ivec3 block_world_pos;
  physics::Ray::Face ray_face;

  chunk_world_pos.y = 0.0f;

  for (const auto &c : ray_chunks) {
    if (!c)
      continue;

    const auto aabb(c->to_aabb());
    if (ray.cast(aabb, ray_face) >= 0.0f) {
      chunk_world_pos.x = static_cast<float>(c->get_position().x);
      chunk_world_pos.z = static_cast<float>(c->get_position().y);

      // Loop over all blocks of chunk
      for (int x = 0; x < block_width; x++) {
        for (int y = 0; y < block_height; y++) {
          for (int z = 0; z < block_depth; z++) {
            const auto &b = c->get_block(x, y, z);
            if (b.type != block::Type::AIR) {
              const glm::vec3 block_pos(
                  chunk_world_pos.x + static_cast<float>(x),
                  static_cast<float>(y),
                  chunk_world_pos.z + static_cast<float>(z));

              if (abs(ray.origin.x - block_pos.x) > raycast_distance ||
                  abs(ray.origin.y - block_pos.y) > raycast_distance ||
                  abs(ray.origin.z - block_pos.z) > raycast_distance) {
                continue;
              }

              const auto t{ray.cast(b.to_aabb(block_pos), ray_face)};

              if (t >= 0.0f && t < distance) {
                distance = t;

                block_world_pos.x = static_cast<int>(block_pos.x);
                block_world_pos.y = y;
                block_world_pos.z = static_cast<int>(block_pos.z);
                face = ray_face;
              }
            }
          }
        }
      }
    }
  }

  // If we intersected with any block
  if (distance != std::numeric_limits<float>::max()) {
    return block_world_pos;
  }

  return std::nullopt;
}

void World::render(const ::core::vulkan::RenderCall &render_call) {
  std::lock_guard lk(m_chunks_mutex);

  m_chunks_to_delete.clear();

  for (auto &[pos, chunk] : m_chunks) {
    chunk->render(render_call);
  }
}

void World::start_update_thread() {
  m_running = true;
  m_chunk_update_thread =
      std::make_unique<std::thread>(std::bind(&World::_update, this));
}

void World::wait_for_generation(const size_t chunk_count) {
  size_t chunks_generated{0};

  m_chunks_mutex.lock();
  while (chunks_generated < chunk_count) {
    for (auto &[_, chunk] : m_chunks) {
      chunks_generated += chunk->check_mesh();
    }
    m_chunks_mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<size_t>(
        1.0f / static_cast<float>(generation_wait_fps) * 1000.0f)));
    m_chunks_mutex.lock();
  }
  m_chunks_mutex.unlock();

  core::Log::info("Generated " + std::to_string(chunks_generated) +
                  " chunks while waiting");
}

std::optional<int> World::get_height(const glm::vec3 &position) {
  const auto pos(get_chunk_position(position));

  std::lock_guard lk(m_chunks_mutex);
  if (m_chunks.find(pos) == m_chunks.end()) {
    return std::nullopt;
  }

  const auto &chunk = m_chunks.at(pos);
  return chunk->get_height(position);
}

void World::clear_and_reseed() {
  std::lock_guard lk(m_chunks_mutex);
  m_chunks.clear();
  m_world_generation.seed(time(nullptr));
}

std::filesystem::path World::world_save_folder;

bool World::_chunks_to_update_contains(
    const std::vector<std::weak_ptr<Chunk>> &chunks_to_update,
    std::shared_ptr<Chunk> chunk) {
  if (!chunk) {
    return true;
  }

  for (auto &_c : chunks_to_update) {
    if (auto c = _c.lock(); c) {
      if (c == chunk) {
        return true;
      }
    }
  }

  return false;
}

std::optional<std::shared_ptr<Chunk>>
World::_get_chunk(const std::pair<int, int> &pos) {
  if (m_chunks.find(pos) == m_chunks.end()) {
    return std::nullopt;
  }

  return m_chunks[pos];
}

const std::optional<const std::shared_ptr<Chunk>>
World::_get_chunk(const std::pair<int, int> &pos) const {
  if (m_chunks.find(pos) == m_chunks.end()) {
    return std::nullopt;
  }

  return m_chunks.at(pos);
}

std::shared_ptr<Chunk> World::_check_if_add_neighbour(
    std::pair<int, int> pos, std::shared_ptr<Chunk> chunk,
    const std::pair<int, int> &center_position, const int x, const int y) {
  pos.first += x;
  pos.second += y;

  if (abs(pos.first - center_position.first) > m_render_distance ||
      abs(pos.second - center_position.second) > m_render_distance) {
    return nullptr;
  }

  auto new_chunk = std::make_shared<Chunk>(m_context, get_world_position(pos));

  // right
  if (x == 1 && y == 0) {
    chunk->set_right(new_chunk);
    new_chunk->set_left(chunk);
    if (auto front = chunk->get_front(); front) {
      if (auto right = front->get_right(); right) {
        right->set_back(new_chunk);
        new_chunk->set_front(right);
      }
    }
    if (auto back = chunk->get_back(); back) {
      if (auto right = back->get_right(); right) {
        right->set_front(new_chunk);
        new_chunk->set_back(right);
      }
    }
    // left
  } else if (x == -1 && y == 0) {
    chunk->set_left(new_chunk);
    new_chunk->set_right(chunk);
    if (auto front = chunk->get_front(); front) {
      if (auto left = front->get_left(); left) {
        left->set_back(new_chunk);
        new_chunk->set_front(left);
      }
    }
    if (auto back = chunk->get_back(); back) {
      if (auto left = back->get_left(); left) {
        left->set_front(new_chunk);
        new_chunk->set_back(left);
      }
    }
    // back
  } else if (x == 0 && y == 1) {
    chunk->set_back(new_chunk);
    new_chunk->set_front(chunk);
    if (auto right = chunk->get_right(); right) {
      if (auto back = right->get_back(); back) {
        back->set_left(new_chunk);
        new_chunk->set_right(back);
      }
    }
    if (auto left = chunk->get_left(); left) {
      if (auto back = left->get_back(); back) {
        back->set_right(new_chunk);
        new_chunk->set_left(back);
      }
    }
    // front
  } else if (x == 0 && y == -1) {
    chunk->set_front(new_chunk);
    new_chunk->set_back(chunk);
    if (auto right = chunk->get_right(); right) {
      if (auto front = right->get_front(); front) {
        front->set_left(new_chunk);
        new_chunk->set_right(front);
      }
    }
    if (auto left = chunk->get_left(); left) {
      if (auto front = left->get_front(); front) {
        front->set_right(new_chunk);
        new_chunk->set_left(front);
      }
    }
  }

  return new_chunk;
}

void World::_update() {
  ::core::FPSTimer timer(update_wait_fps);

  while (m_running) {
    auto delta_timer(timer.begin_frame());

#ifndef NDEBUG
    const auto update_start_time = std::chrono::high_resolution_clock::now();
#endif

    m_center_position_mutex.lock();
    const auto center_position(get_chunk_position(m_center_position));
    m_center_position_mutex.unlock();

    // First remove all chunks that are too far away
    std::set<std::pair<int, int>> chunks_to_remove;
    {
      std::lock_guard lk(m_chunks_mutex);
      for (const auto &[pos, chunk] : m_chunks) {
        if (abs(pos.first - center_position.first) > m_render_distance ||
            abs(pos.second - center_position.second) > m_render_distance) {
          chunks_to_remove.emplace(pos);
        }
      }
    }

    if (!chunks_to_remove.empty()) {
      std::lock_guard lk(m_chunks_mutex);
      for (const auto &pos : chunks_to_remove) {
        auto &chunk = m_chunks[pos];

        auto stored_blocks(chunk->to_stored_blocks());
        m_save_world->store_chunk(pos, stored_blocks);

        m_chunks_to_delete.emplace_back(chunk);
        m_chunks.erase(pos);
      }
    }

#ifndef NDEBUG
    size_t chunks_added{0};
#endif

    std::vector<std::weak_ptr<Chunk>> chunks_to_update;
    {
      std::lock_guard lk(m_chunks_mutex);
      if (m_chunks.empty()) {
#ifndef NDEBUG
        chunks_added++;
#endif

        auto chunk = std::make_shared<Chunk>(
            m_context, get_world_position(center_position));
        const auto stored_blocks(m_save_world->load_chunk(center_position));
        if (stored_blocks) {
          chunk->from_stored_blocks(*stored_blocks);
        } else {
          chunk->from_world_generation(m_world_generation);
          m_save_world->store_chunk(center_position, chunk->to_stored_blocks());
        }

        m_chunks.emplace(center_position, chunk);
        chunks_to_update.emplace_back(chunk);
      }
    }

    // Loop over all chunks and check if neighbors should be added
    // NOTE: m_chunks is accessed without locking a mutex. This could lead to
    // race conditions
    std::vector<std::shared_ptr<Chunk>> chunks_to_add;
    for (const auto &[pos, chunk] : m_chunks) {
      if (!chunk->get_right()) {
        // right
        chunks_to_add.emplace_back(
            _check_if_add_neighbour(pos, chunk, center_position, 1, 0));
      }
      if (!chunk->get_left()) {
        // left
        chunks_to_add.emplace_back(
            _check_if_add_neighbour(pos, chunk, center_position, -1, 0));
      }
      if (!chunk->get_front()) {
        // front
        chunks_to_add.emplace_back(
            _check_if_add_neighbour(pos, chunk, center_position, 0, -1));
      }
      if (!chunk->get_back()) {
        // back
        chunks_to_add.emplace_back(
            _check_if_add_neighbour(pos, chunk, center_position, 0, 1));
      }
    }

    {
      std::lock_guard lk(m_chunks_mutex);
      for (auto &chunk : chunks_to_add) {
        if (!chunk)
          continue;

#ifndef NDEBUG
        chunks_added++;
#endif

        const auto chunk_pos(get_chunk_position(chunk->get_position()));
        const auto stored_blocks(m_save_world->load_chunk(chunk_pos));

        if (stored_blocks) {
          chunk->from_stored_blocks(*stored_blocks);
        } else {
          chunk->from_world_generation(m_world_generation);
          m_save_world->store_chunk(chunk_pos, chunk->to_stored_blocks());
        }

        m_chunks.emplace(get_chunk_position(chunk->get_position()), chunk);
        chunks_to_update.emplace_back(chunk);
      }
    }

    // Update neighbouring chunks
    for (auto &_chunk : chunks_to_update) {
      if (auto chunk = _chunk.lock(); chunk) {
        chunk->needs_face_update();
        chunk->generate(m_block_server);
      }
    }

#ifndef NDEBUG
    if (!(chunks_added == 0 && chunks_to_update.empty() &&
          chunks_to_remove.empty())) {
      std::lock_guard lk(m_chunks_mutex);
      const auto gen_end_time = std::chrono::high_resolution_clock::now();
      {
        std::stringstream stream;
        stream << "chunk::World::Update: +" << chunks_added << " -"
               << chunks_to_remove.size() << " *"
               << (chunks_to_update.size() - chunks_added) << " ="
               << m_chunks.size();
        stream << ' '
               << std::chrono::duration_cast<std::chrono::microseconds>(
                      gen_end_time - update_start_time)
                      .count()
               << " µs";
        ::core::Log::debug(stream.str());
      }
    }
#endif
  }
}
} // namespace chunk
