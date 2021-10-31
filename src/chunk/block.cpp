#include "block.hpp"

namespace chunk {
Vertex::Vertex(float x, float y, float z, float u, float v, float _light)
    : position(x, y, z), uv(u, v), light(_light) {}

Block::Block()
    : type(block::Type::AIR), m_faces(0), m_light(block::Light::MAX) {}

void Block::generate(const block::Server &block_server,
                     std::vector<Vertex> &vertices,
                     std::vector<uint32_t> &indices,
                     const glm::vec3 &position) const {
  const auto &tex_coords = block_server.get_texture_coordinates(type);

  _create_cube(vertices, indices, position, tex_coords, light(), front_face(),
               back_face(), left_face(), right_face(), top_face(), bot_face());
}

physics::AABB Block::to_aabb(const glm::vec3 &position) const {
  return physics::AABB(position.x, position.y, position.z, 1.0f, 1.0f, 1.0f);
}

void Block::_create_cube(std::vector<Vertex> &vertices,
                         std::vector<uint32_t> &indices, const glm::vec3 &p,
                         const block::Server::TextureCoordinates &tex_coords,
                         const float light, const bool front_face,
                         const bool back_face, const bool left_face,
                         const bool right_face, const bool top_face,
                         const bool bot_face) {
  if (!(front_face || back_face || left_face || right_face || top_face ||
        bot_face))
    return;

  if (front_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.front.x, tex_coords.front.w, light); // 0
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.front.z, tex_coords.front.w, light); // 1
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.front.z, tex_coords.front.y, light); // 2
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.front.x, tex_coords.front.y, light); // 3

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 0);
  }

  if (back_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.back.x, tex_coords.back.y, light); // 4
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.back.z, tex_coords.back.y, light); // 5
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.back.z, tex_coords.back.w, light); // 6
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.back.x, tex_coords.back.w, light); // 7

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
  }

  if (right_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.right.x, tex_coords.right.y,
                          light); // 10
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.right.z, tex_coords.right.w,
                          light); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.right.z, tex_coords.right.y,
                          light); // 16
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.right.x, tex_coords.right.w,
                          light); // 17

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 1);
  }

  if (left_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.left.z, tex_coords.left.y, light); // 11
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f,

                          tex_coords.left.x, tex_coords.left.w, light); // 12
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.left.x, tex_coords.left.y, light); // 18
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.left.z, tex_coords.left.w, light); // 19

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 2);
  }

  if (top_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, tex_coords.top.x,
                          tex_coords.top.w, light); // 8
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, tex_coords.top.z,
                          tex_coords.top.w, light); // 9
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, tex_coords.top.z,
                          tex_coords.top.y, light); // 10
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.top.x, tex_coords.top.y, light); // 11

    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 0);
  }

  if (bot_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.bot.x, tex_coords.bot.y, light); // 12
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, tex_coords.bot.z,
                          tex_coords.bot.y, light); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.bot.z, tex_coords.bot.w, light); // 14
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.bot.x, tex_coords.bot.w, light); // 15

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 3);
  }
}

void BlockArray::fill(const block::Type value) {
  for (auto &b : m_array) {
    b.type = value;
  }
}

void BlockArray::half_fill(const block::Type value) {
  for (int x = 0; x < block_width; x++) {
    for (int z = 0; z < block_depth; z++) {
      for (int y = 0; y < block_height / 2; y++) {
        set(x, y, z, value);
      }
    }
  }
}

void BlockArray::clear() { fill(block::Type::AIR); }

std::array<uint8_t, block_width * block_depth * block_height>
BlockArray::to_stored_blocks() const {
  std::array<uint8_t, block_width * block_depth * block_height> stored_blocks;

  for (int x = 0; x < block_width; x++) {
    for (int z = 0; z < block_depth; z++) {
      for (int y = 0; y < block_height; y++) {
        stored_blocks[x + z * block_width + y * (block_width * block_depth)] =
            static_cast<uint8_t>(get(x, y, z));
      }
    }
  }

  return stored_blocks;
}

void BlockArray::from_stored_blocks(
    const std::array<uint8_t, block_width * block_depth * block_height>
        &stored_blocks) {
  for (int x = 0; x < block_width; x++) {
    for (int z = 0; z < block_depth; z++) {
      for (int y = 0; y < block_height; y++) {
        set(x, y, z,
            static_cast<block::Type>(
                stored_blocks[x + z * block_width +
                              y * (block_width * block_depth)]));
      }
    }
  }
}

} // namespace chunk
