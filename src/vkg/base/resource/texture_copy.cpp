#include "texture_copy.hpp"
#include "texture_layout.hpp"

namespace vkg::image {
auto copy(vk::CommandBuffer cb, vk::Image dstImage, Texture &srcTexture) -> void {
    vk::ImageCopy region;
    region.srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.dstSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.extent = srcTexture.extent();
    cb.copyImage(
        srcTexture.image(), vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal,
        region);
}
auto copy(
    vk::CommandBuffer cb, Texture &dstTexture, vk::ImageSubresource subresource, vk::Extent3D extent, vk::Buffer buffer,
    uint32_t offset) -> void {
    transitTo(
        cb, dstTexture, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eTransferWrite,
        vk::PipelineStageFlagBits::eTransfer);
    vk::BufferImageCopy region;
    region.bufferOffset = offset;
    region.imageSubresource = {subresource.aspectMask, subresource.mipLevel, subresource.arrayLayer, 1};
    region.imageExtent = extent;
    cb.copyBufferToImage(buffer, dstTexture.image(), vk::ImageLayout::eTransferDstOptimal, region);
}
auto copy(vk::CommandBuffer cb, vk::Buffer dstBuffer, Texture &srcTexture) -> void {
    vk::BufferImageCopy copyRegion{};
    copyRegion.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    copyRegion.imageExtent = srcTexture.extent();
    cb.copyImageToBuffer(srcTexture.image(), srcTexture.layout(), dstBuffer, copyRegion);
}
auto copy(vk::CommandBuffer cb, Texture &dstTexture, vk::Buffer srcBuffer) -> void {
    vk::BufferImageCopy copyRegion{};
    copyRegion.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    copyRegion.imageExtent = dstTexture.extent();
    cb.copyBufferToImage(srcBuffer, dstTexture.image(), dstTexture.layout(), copyRegion);
}
auto copy(vk::CommandBuffer cb, Texture &dstTexture, Texture &srcTexture) -> void {
    vk::ImageCopy region;
    region.srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.dstSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.extent = srcTexture.extent();
    cb.copyImage(
        srcTexture.image(), vk::ImageLayout::eTransferSrcOptimal, dstTexture.image(),
        vk::ImageLayout::eTransferDstOptimal, region);
}
}