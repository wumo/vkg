#include "texture_mipmap.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include "texture_layout.hpp"

namespace vkg::image {
auto generateMipmap(uint32_t queueIdx, Texture &texture) -> void {
  auto &device = texture.device();
  auto formatProp = device.physicalDevice().getFormatProperties(texture.format());
  errorIf(
    !(formatProp.optimalTilingFeatures & vk::FormatFeatureFlagBits ::eBlitSrc) ||
      !(formatProp.optimalTilingFeatures & vk::FormatFeatureFlagBits ::eBlitDst),
    "blit feature src/dst not satisfied!!");
  errorIf(
    !(formatProp.optimalTilingFeatures &
      vk::FormatFeatureFlagBits ::eSampledImageFilterLinear),
    "texture image format does not support linear blitting!");

  device.execSync(
    [&](vk::CommandBuffer cb) {
      int32_t mipWidth = texture.extent().width;
      int32_t mipHeight = texture.extent().height;

      for(uint32_t level = 1; level < texture.mipLevels(); ++level) {
        setLayout(
          cb, texture.image(), vk::ImageLayout::eTransferDstOptimal,
          vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferWrite,
          vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eAllCommands,
          vk::PipelineStageFlagBits::eAllCommands, level - 1);
        vk::ImageBlit blit{
          {vk::ImageAspectFlagBits::eColor, level - 1, 0, 1},
          {vk::Offset3D{}, {mipWidth, mipHeight, 1}},
          {vk::ImageAspectFlagBits::eColor, level, 0, 1},
          {vk::Offset3D{},
           {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}};
        cb.blitImage(
          texture.image(), vk::ImageLayout::eTransferSrcOptimal, texture.image(),
          vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
        if(mipWidth > 1) mipWidth /= 2;
        if(mipHeight > 1) mipHeight /= 2;
      }
      setLayout(
        cb, texture.image(), vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, texture.mipLevels() - 1);
      setLayout(
        cb, texture.image(), vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eTransferRead,
        vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, 0, texture.mipLevels(), 0,
        texture.arrayLayers());
      texture.recordLayout(
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
        vk::PipelineStageFlagBits::eAllCommands);
    },
    queueIdx);
}
}