#pragma once

namespace block {

enum Type {
  AIR,
  GRASS,
  DIRT,
};

enum Light {
  MIN = 10,
  MAX = 255,
  STEPS = 10,
};

constexpr const char *type_as_str(const Type t) {
  switch (t) {
  case Type::AIR:
    return "AIR";
  case Type::GRASS:
    return "GRASS";
  case Type::DIRT:
    return "DIRT";
  default:
    return "INVALID";
  }
}

} // namespace block
