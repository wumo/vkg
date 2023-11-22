#pragma once
#include "texture_creator.hpp"

namespace vkg::image {
auto copy(vk::CommandBuffer cb, vk::Image dstTexture, Texture &srcTexture) -> void;
auto copy(
    vk::CommandBuffer cb, Texture &dstTexture, vk::ImageSubresource subresource, vk::Extent3D extent, vk::Buffer buffer,
    uint32_t offset) -> void;
auto copy(vk::CommandBuffer cb, vk::Buffer dstBuffer, Texture &srcTexture) -> void;
auto copy(vk::CommandBuffer cb, Texture &dstTexture, vk::Buffer srcBuffer) -> void;
auto copy(vk::CommandBuffer cb, Texture &dstTexture, Texture &srcTexture) -> void;
}
