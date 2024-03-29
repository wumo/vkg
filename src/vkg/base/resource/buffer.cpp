#include "buffer.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
Buffer::Buffer(Device &device, vk::BufferCreateInfo info, VmaAllocationCreateInfo allocInfo, const std::string &name) {
    this->info = info;
    vmaBuffer = UniquePtr(new VmaBuffer{device}, [=](VmaBuffer *ptr) {
        debugLog("deallocate buffer:", name, " ", VkBuffer(ptr->buffer));
        vmaDestroyBuffer(ptr->vkezDevice.allocator(), VkBuffer(ptr->buffer), ptr->allocation);
        delete ptr;
    });
    auto result = vmaCreateBuffer(
        device.allocator(), (VkBufferCreateInfo *)&info, &allocInfo, reinterpret_cast<VkBuffer *>(&(vmaBuffer->buffer)),
        &vmaBuffer->allocation, &alloc);
    errorIf(result != VK_SUCCESS, "failed to allocate buffer!");
    debugLog(
        "allocate buffer:", name, " ", VkBuffer(vmaBuffer->buffer), "[", alloc.deviceMemory, "+", alloc.offset, "]");
    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(device.allocator(), alloc.memoryType, &memFlags);
    if((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        mappable = true;
    }
    device.name(bufferInfo().buffer, name);
}

void Buffer::barrier(
    vk::CommandBuffer &cb, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::DependencyFlags dependencyFlags,
    uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) {
    vk::BufferMemoryBarrier barrier{
        srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, vmaBuffer->buffer, 0, alloc.size};
    cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, nullptr, barrier, nullptr);
}
auto Buffer::bufferInfo() const -> BufferInfo { return {vmaBuffer->buffer, 0, info.size}; }
auto Buffer::device() const -> Device & { return vmaBuffer->vkezDevice; }
auto Buffer::devMem() const -> std::pair<vk::DeviceMemory, vk::DeviceSize> {
    return {vk::DeviceMemory{alloc.deviceMemory}, alloc.offset};
}
auto Buffer::size() const -> vk::DeviceSize { return info.size; }
}
