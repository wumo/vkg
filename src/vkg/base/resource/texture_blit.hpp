#pragma once
#include "texture_creator.hpp"

namespace vkg::image {
auto blit(
    vk::CommandBuffer cb, vk::Image dstTexture, std::array<vk::Offset3D, 2> dstOffsets, Texture &srcTexture,
    std::array<vk::Offset3D, 2> srcOffsets) -> void;
}
