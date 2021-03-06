#pragma once
#include <cstdint>

namespace vkg {
class Mesh {
public:
  Mesh(uint32_t id, uint32_t primitive, uint32_t material);
  auto id() const -> uint32_t;
  auto primitive() const -> uint32_t;
  auto material() const -> uint32_t;

protected:
  const uint32_t id_;
  const uint32_t primitive_;
  const uint32_t material_;
};
}