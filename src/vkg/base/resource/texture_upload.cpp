#include "texture_upload.hpp"
#include "texture_layout.hpp"
#include "texture_copy.hpp"

namespace vkg::image {
auto upload(Texture &texture, std::span<std::byte> bytes, bool transitToShaderRead)
  -> void {
  auto &device = texture.device();
  auto stagingBuffer =
    buffer::hostBuffer(device, vk::BufferUsageFlagBits::eTransferSrc, bytes.size_bytes());
  buffer::updateBytes(*stagingBuffer, bytes.data(), bytes.size_bytes());
  device.execSyncInGraphicsQueue([&](vk::CommandBuffer cb) {
    copy(
      cb, texture, {vk::ImageAspectFlagBits::eColor, 0, 0}, texture.extent(),
      stagingBuffer->buffer(), 0);
    if(transitToShaderRead)
      transitTo(
        cb, texture, vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eAllCommands);
  });
}
}
