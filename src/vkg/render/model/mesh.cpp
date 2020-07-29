#include "mesh.hpp"

namespace vkg {
Mesh::Mesh(uint32_t id, uint32_t primitive, uint32_t material)
  : id_{id}, primitive_{primitive}, material_{material} {}
auto Mesh::id() const -> uint32_t { return id_; }
auto Mesh::primitive() const -> uint32_t { return primitive_; }
auto Mesh::material() const -> uint32_t { return material_; }
}