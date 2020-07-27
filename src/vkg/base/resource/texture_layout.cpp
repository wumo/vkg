#include "texture_layout.hpp"

namespace vkg::image {
void setLayout(
  vk::CommandBuffer cb, vk::Image image, vk::ImageLayout oldLayout,
  vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
  vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
  uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer,
  uint32_t layerCount) {
  if(oldLayout == newLayout) return;

  auto aspectMask = newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal ?
                      vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil :
                      vk::ImageAspectFlagBits::eColor;

  vk::ImageMemoryBarrier imageMemoryBarrier{
    srcAccessMask,
    dstAccessMask,
    oldLayout,
    newLayout,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    image,
    {aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount}};
  cb.pipelineBarrier(
    srcStageMask, dstStageMask, {}, nullptr, nullptr, imageMemoryBarrier);
}

auto transitTo(
  vk::CommandBuffer cb, Texture &texture, vk::ImageLayout newLayout,
  vk::AccessFlags dstAccess, vk::PipelineStageFlags dstStage) -> void {
  setLayout(
    cb, texture.image(), texture.layout(), newLayout, texture.accessFlag(), dstAccess,
    texture.stageFlag(), dstStage, 0, texture.mipLevels(), 0, texture.arrayLayers());
  texture.recordLayout(newLayout, dstAccess, dstStage);
}
}