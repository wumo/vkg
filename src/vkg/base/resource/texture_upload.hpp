#pragma once
#include "texture.hpp"
#include "buffers.hpp"
#include <span>

namespace vkg::image {
auto upload(Texture &texture, std::span<std::byte> bytes, bool transitToShaderRead = true)
  -> void;
}
