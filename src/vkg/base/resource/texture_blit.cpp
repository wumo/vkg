#include "texture_blit.hpp"
#include "texture_layout.hpp"

namespace vkg::image {
auto blit(
    vk::CommandBuffer cb, vk::Image dstImage, std::array<vk::Offset3D, 2> dstOffsets, Texture &srcTexture,
    std::array<vk::Offset3D, 2> srcOffsets) -> void {
    vk::ImageBlit region{
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, srcOffsets, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, dstOffsets};
    cb.blitImage(
        srcTexture.image(), vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal,
        region, vk::Filter::eLinear);
}
}