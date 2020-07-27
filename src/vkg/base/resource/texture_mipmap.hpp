#pragma once
#include "texture.hpp"

namespace vkg::image {
auto generateMipmap(Texture &texture) -> void;
}
