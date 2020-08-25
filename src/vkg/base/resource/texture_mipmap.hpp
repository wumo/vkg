#pragma once
#include "texture.hpp"

namespace vkg::image {
auto generateMipmap(uint32_t queueIdx,Texture &texture) -> void;
}
