#include "texture.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
Texture::Texture(
  Device &device, vk::ImageCreateInfo info, VmaAllocationCreateInfo allocInfo,
  const std::string &name) {
  currentLayout = info.initialLayout;
  this->info = info;
  vmaImage = UniquePtr(new VmaImage{device}, [](VmaImage *ptr) {
    debugLog("deallocate image:", ptr->image);
    vmaDestroyImage(ptr->vkezDevice.allocator(), ptr->image, ptr->allocation);
    delete ptr;
  });
  auto result = vmaCreateImage(
    device.allocator(), reinterpret_cast<VkImageCreateInfo *>(&info), &allocInfo,
    reinterpret_cast<VkImage *>(&(vmaImage->image)), &vmaImage->allocation, &alloc);
  errorIf(result != VK_SUCCESS, "failed to allocate image!");
  debugLog(
    "allocate image: ", name, " ", vmaImage->image, "[", alloc.deviceMemory, "+",
    alloc.offset, "]");
  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(device.allocator(), alloc.memoryType, &memFlags);
  if(
    (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
    (memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    mappable = true;
  }
}

auto Texture::barrier(
  vk::ImageLayout newLayout, const vk::AccessFlags &dstAccess,
  vk::PipelineStageFlagBits dstStage, uint32_t srcQueueFamilyIndex,
  uint32_t dstQueueFamilyIndex, vk::ImageAspectFlagBits aspect)
  -> vk::ImageMemoryBarrier {
  vk::ImageMemoryBarrier barrier_{
    srcAccess,           dstAccess,
    currentLayout,       newLayout,
    srcQueueFamilyIndex, dstQueueFamilyIndex,
    vmaImage->image,     {aspect, 0, info.mipLevels, 0, info.arrayLayers}};
  recordLayout(newLayout, dstAccess, dstStage);
  return barrier_;
}
void Texture::setImageView(
  vk::ImageViewType viewType, const vk::ImageAspectFlags &aspectMask) {
  vk::ImageViewCreateInfo viewCreateInfo{
    {},
    vmaImage->image,
    viewType,
    info.format,
    {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
     vk::ComponentSwizzle::eA},
    {aspectMask, 0, info.mipLevels, 0, info.arrayLayers}};
  viewType_ = viewType;
  aspect_ = aspectMask;
  imageView_ = vmaImage->vkezDevice.vkDevice().createImageViewUnique(viewCreateInfo);
}
auto Texture::createLayerImageView(uint32_t layer) -> vk::UniqueImageView {
  vk::ImageViewCreateInfo viewCreateInfo{
    {},
    vmaImage->image,
    viewType_,
    info.format,
    {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
     vk::ComponentSwizzle::eA},
    {aspect_, 0, info.mipLevels, layer, 1}};
  return vmaImage->vkezDevice.vkDevice().createImageViewUnique(viewCreateInfo);
}
auto Texture::setSampler(const vk::SamplerCreateInfo &samplerCreateInfo) -> void {
  sampler_ = device().vkDevice().createSamplerUnique(samplerCreateInfo);
}
auto Texture::image() const -> vk::Image { return vmaImage->image; }
auto Texture::format() const -> vk::Format { return info.format; }
auto Texture::sampler() const -> vk::Sampler { return *sampler_; }
auto Texture::imageView() const -> vk::ImageView { return *imageView_; }
auto Texture::device() const -> Device & { return vmaImage->vkezDevice; }
auto Texture::extent() const -> vk::Extent3D { return info.extent; }
auto Texture::layout() const -> vk::ImageLayout { return currentLayout; }
auto Texture::accessFlag() const -> vk::AccessFlags { return srcAccess; }
auto Texture::stageFlag() const -> vk::PipelineStageFlags { return srcStage; }
auto Texture::mipLevels() const -> uint32_t { return info.mipLevels; }
auto Texture::arrayLayers() const -> uint32_t { return info.arrayLayers; }
auto Texture::recordLayout(
  vk::ImageLayout layout, vk::AccessFlags access, vk::PipelineStageFlags stage) -> void {
  currentLayout = layout;
  srcAccess = access;
  srcStage = stage;
}
auto Texture::aspectFlag() const -> vk::ImageAspectFlags { return aspect_; }
}