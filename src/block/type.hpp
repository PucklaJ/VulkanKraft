#pragma once

namespace block {

enum Type {
  AIR,
  GRASS,
  DIRT,
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
