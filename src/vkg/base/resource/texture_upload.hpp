#pragma once
#include "texture.hpp"
#include "buffers.hpp"
#include <span>

namespace vkg::image {
void upload(
  uint32_t queueIdx, Texture &texture, std::span<std::byte> bytes,
  bool transitToShaderRead = true);
}
