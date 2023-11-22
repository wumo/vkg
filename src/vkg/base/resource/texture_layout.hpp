#pragma once

#include "texture.hpp"

namespace vkg::image {
auto setLayout(
    vk::CommandBuffer cb, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
    vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands, uint32_t baseMipLevel = 0,
    uint32_t levelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1) -> void;

auto transitTo(
    vk::CommandBuffer cb, Texture &texture, vk::ImageLayout newLayout, vk::AccessFlags dstAccess,
    vk::PipelineStageFlags dstStage) -> void;
}
