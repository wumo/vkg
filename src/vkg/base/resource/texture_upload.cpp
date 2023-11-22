#include "texture_upload.hpp"
#include "texture_layout.hpp"
#include "texture_copy.hpp"

namespace vkg::image {
void upload(uint32_t queueIdx, Texture &texture, std::span<std::byte> bytes, bool transitToShaderRead) {
    auto &device = texture.device();
    auto stagingBuffer = buffer::hostBuffer(device, vk::BufferUsageFlagBits::eTransferSrc, bytes.size_bytes());
    buffer::updateBytes(*stagingBuffer, bytes.data(), bytes.size_bytes());
    device.execSync(
        [&](vk::CommandBuffer cb) {
            copy(
                cb, texture, {vk::ImageAspectFlagBits::eColor, 0, 0}, texture.extent(),
                stagingBuffer->bufferInfo().buffer, 0);
            if(transitToShaderRead)
                transitTo(
                    cb, texture, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
                    vk::PipelineStageFlagBits::eAllCommands);
        },
        queueIdx);
}
}
