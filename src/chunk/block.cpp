#include "block.hpp"

namespace chunk {
Vertex::Vertex(float x, float y, float z, float u, float v)
    : position(x, y, z), uv(u, v) {}

Block::Block() { std::fill(m_faces.begin(), m_faces.end(), false); }

void Block::generate(std::vector<Vertex> &vertices,
                     std::vector<uint32_t> &indices,
                     const glm::vec3 &position) const {
  _create_cube(vertices, indices, position, m_faces[front_face_index],
               m_faces[back_face_index], m_faces[left_face_index],
               m_faces[right_face_index], m_faces[top_face_index],
               m_faces[bot_face_index]);
}

::core::math::AABB Block::to_aabb(const glm::vec3 &p) const {
  ::core::math::AABB aabb;

  aabb.min = p;

  aabb.max = aabb.min + glm::vec3(1.0f, 1.0f, 1.0f);

  return aabb;
}

void Block::_create_cube(std::vector<Vertex> &vertices,
                         std::vector<uint32_t> &indices, const glm::vec3 &p,
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

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f,
                          1.0f);                                            // 0
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f, 1.0f); // 1
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 0.0f);  // 2
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 0.0f); // 3

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

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f,
                          1.0f); // 4
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
                          1.0f);                                            // 5
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f, 0.0f); // 6
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f,
                          0.0f); // 7

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

    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f,
                          0.0f); // 10
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f,
                          1.0f); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f,
                          1.0f);                                           // 16
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 0.0f); // 17

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

    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f,
                          0.0f); // 11
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f,
                          1.0f); // 12
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
                          1.0f); // 18
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f,
                          0.0f); // 19

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

    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + 0.5f, 0.0f, 1.0f); // 8
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + 0.5f, 1.0f, 1.0f);  // 9
    vertices.emplace_back(p.x + 0.5f, p.y + 0.5f, p.z + -0.5f, 1.0f,
                          0.0f); // 10
    vertices.emplace_back(p.x + -0.5f, p.y + 0.5f, p.z + -0.5f, 0.0f,
                          0.0f); // 11

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

    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + 0.5f, 1.0f,
                          1.0f); // 12
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + 0.5f, 0.0f,
                          1.0f); // 13
    vertices.emplace_back(p.x + 0.5f, p.y + -0.5f, p.z + -0.5f, 0.0f,
                          0.0f); // 14
    vertices.emplace_back(p.x + -0.5f, p.y + -0.5f, p.z + -0.5f, 1.0f,
                          0.0f); // 15

    indices.reserve(indices.size() + indices_per_face);
    indices.emplace_back(i + 3);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 3);
  }
}

void BlockArray::fill(const BlockType value) {
  for (auto &b : m_array) {
    b.type = value;
  }
}

void BlockArray::clear() { fill(BlockType::AIR); }
} // namespace chunk
