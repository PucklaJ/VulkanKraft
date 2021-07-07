#include "block.hpp"

namespace chunk {
Vertex::Vertex(float x, float y, float z, float u, float v)
    : position(x, y, z), uv(u, v) {}

Block::Block() : type(block::Type::AIR) {
  std::fill(m_faces.begin(), m_faces.end(), false);
}

void Block::generate(const block::Server &block_server,
                     std::vector<Vertex> &vertices,
                     std::vector<uint32_t> &indices,
                     const glm::vec3 &position) const {
  const auto &tex_coords = block_server.get_texture_coordinates(type);

  _create_cube(vertices, indices, position, tex_coords,
               m_faces[front_face_index], m_faces[back_face_index],
               m_faces[left_face_index], m_faces[right_face_index],
               m_faces[top_face_index], m_faces[bot_face_index]);
}

::core::math::AABB Block::to_aabb(const glm::vec3 &p) const {
  ::core::math::AABB aabb;

  aabb.min = p;

  aabb.max = aabb.min + glm::vec3(1.0f, 1.0f, 1.0f);

  return aabb;
}

void Block::_create_cube(std::vector<Vertex> &vertices,
                         std::vector<uint32_t> &indices, const glm::vec3 &p,
                         const block::Server::TextureCoordinates &tex_coords,
                         const bool front_face, const bool back_face,
                         const bool left_face, const bool right_face,
                         const bool top_face, const bool bot_face) {
  if (!(front_face || back_face || left_face || right_face || top_face ||
        bot_face))
    return;

  // vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f, 1.0f); //
  // 0 vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f, 1.0f);
  // // 1 vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 0.0f);
  // // 2 vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f,
  // 0.0f);   // 3 vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z +
  // -0.5f, 1.0f, 1.0f); // 4 vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z
  // + -0.5f, 0.0f, 1.0f);  // 5 vertices.emplace_back(p.x + 0.5f, p.y + 0.5f,
  // p.z + -0.5f, 0.0f, 0.0f);   // 6 vertices.emplace_back(p.x + -0.5f, p.y +
  // 0.5f, p.z + -0.5f, 1.0f, 0.0f);  // 7 vertices.emplace_back(p.x + -0.5f,
  // p.y + 0.5f, p.z + 0.5f, 0.0f, 1.0f);   // 8 vertices.emplace_back(p.x +
  // 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 1.0f);    // 9
  // vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f, 0.0f); //
  // 10 vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f,
  //                       0.0f);                                             //
  //                       11
  // vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f, 1.0f); //
  // 12 vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f, 1.0f);
  // // 13 vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
  // 0.0f); // 14 vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z +
  // -0.5f, 1.0f,
  //                       0.0f);                                             //
  //                       15
  // vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f, 1.0f); //
  // 16 vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 0.0f);
  // // 17 vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
  //                       1.0f);                                            //
  //                       18
  // vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 0.0f); //
  // 19

  if (front_face) {
    const auto i{vertices.size()};
    vertices.reserve(vertices.size() + vertices_per_face);

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.front.x,
                          tex_coords.front.w); // 0
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.front.z, tex_coords.front.w); // 1
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.front.z, tex_coords.front.y); // 2
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.front.x, tex_coords.front.y); // 3

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
                          tex_coords.back.z,
                          tex_coords.back.w); // 4
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.back.x,
                          tex_coords.back.w); // 5
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.back.x, tex_coords.back.y); // 6
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.back.z,
                          tex_coords.back.y); // 7

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
                          tex_coords.right.z,
                          tex_coords.right.y); // 10
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.right.x,
                          tex_coords.right.w); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.right.z,
                          tex_coords.right.w); // 16
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.right.x, tex_coords.right.y); // 17

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
                          tex_coords.left.x,
                          tex_coords.left.y); // 11
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f,
                          tex_coords.left.z,
                          tex_coords.left.w); // 12
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.left.x,
                          tex_coords.left.w); // 18
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f,
                          tex_coords.left.z,
                          tex_coords.left.y); // 19

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
                          tex_coords.top.w); // 8
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, tex_coords.top.z,
                          tex_coords.top.w); // 9
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, tex_coords.top.z,
                          tex_coords.top.y); // 10
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f,
                          tex_coords.top.x,
                          tex_coords.top.y); // 11

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
                          tex_coords.bot.z,
                          tex_coords.bot.w); // 12
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, tex_coords.bot.x,
                          tex_coords.bot.w); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.bot.x,
                          tex_coords.bot.y); // 14
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f,
                          tex_coords.bot.z,
                          tex_coords.bot.y); // 15

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
