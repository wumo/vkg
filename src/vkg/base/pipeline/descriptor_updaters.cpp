#include "descriptor_updaters.hpp"

namespace vkg {

DescriptorUpdater::DescriptorUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : layoutMaker{layoutMaker}, setUpdater{setUpdater}, binding{binding} {}

auto DescriptorUpdater::descriptorCount() -> uint32_t & {
  return layoutMaker.descriptorCount(binding);
}

BufferUpdater::BufferUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding, vk::DescriptorType descriptorType)
  : DescriptorUpdater(layoutMaker, setUpdater, binding), descriptorType{descriptorType} {}

auto BufferUpdater::operator()(
  uint32_t dstArrayElement, uint32_t descriptorCount,
  const vk::DescriptorBufferInfo *pBufferInfo) -> void {
  setUpdater.writeBuffers(
    binding, dstArrayElement, descriptorType, descriptorCount, pBufferInfo);
}
auto BufferUpdater::operator()(
  uint32_t dstArrayElement, vk::Buffer buffer, vk::DeviceSize offset,
  vk::DeviceSize range) -> void {
  setUpdater.writeBuffer(
    binding, dstArrayElement, descriptorType, {buffer, offset, range});
}
auto BufferUpdater::operator()(
  vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range) -> void {
  operator()(0, buffer, offset, range);
}

ImageUpdater::ImageUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding, vk::DescriptorType descriptorType)
  : DescriptorUpdater(layoutMaker, setUpdater, binding), descriptorType{descriptorType} {}
auto ImageUpdater::operator()(
  vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout layout) -> void {
  setUpdater.writeImage(binding, 0, descriptorType, {sampler, imageView, layout});
}
auto ImageUpdater::operator()(Texture &texture, vk::ImageLayout layout) -> void {
  operator()(texture.sampler(), texture.imageView(), layout);
}
auto ImageUpdater::operator()(
  uint32_t dstArrayElement, uint32_t descriptorCount,
  const vk::DescriptorImageInfo *pImageInfo) -> void {
  setUpdater.writeImages(
    binding, dstArrayElement, descriptorType, descriptorCount, pImageInfo);
}

BufferViewUpdater::BufferViewUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding, vk::DescriptorType descriptorType)
  : DescriptorUpdater{layoutMaker, setUpdater, binding}, descriptorType{descriptorType} {}
auto BufferViewUpdater::operator()(vk::BufferView texelBufferView) -> void {
  setUpdater.writeBufferView(binding, 0, descriptorType, texelBufferView);
}
auto BufferViewUpdater::operator()(
  uint32_t dstArrayElement, uint32_t descriptorCount,
  const vk::BufferView *pTexelBufferView) -> void {
  setUpdater.writeBufferViews(
    binding, dstArrayElement, descriptorType, descriptorCount, pTexelBufferView);
}

AccelerationStructureUpdater::AccelerationStructureUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : DescriptorUpdater{layoutMaker, setUpdater, binding} {}
auto AccelerationStructureUpdater::operator()(vk::AccelerationStructureNV ASInfo)
  -> void {
  setUpdater.writeAccelerationStructure(binding, 0, ASInfo);
}
auto AccelerationStructureUpdater::operator()(
  uint32_t dstArrayElement, uint32_t descriptorCount,
  const vk::AccelerationStructureNV *pASInfo) -> void {
  setUpdater.writeAccelerationStructures(
    binding, dstArrayElement, descriptorCount, pASInfo);
}

UniformUpdater::UniformUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : BufferUpdater(layoutMaker, setUpdater, binding, vk::DescriptorType::eUniformBuffer){};

UniformDynamicUpdater::UniformDynamicUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : BufferUpdater(
      layoutMaker, setUpdater, binding, vk::DescriptorType::eUniformBufferDynamic){};

StorageBufferUpdater::StorageBufferUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : BufferUpdater(layoutMaker, setUpdater, binding, vk::DescriptorType::eStorageBuffer){};
StorageBufferDynamicUpdater::StorageBufferDynamicUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding, uint32_t descriptorCount)
  : BufferUpdater(
      layoutMaker, setUpdater, binding, vk::DescriptorType::eStorageBufferDynamic){};

SamplerUpdater::SamplerUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : ImageUpdater{layoutMaker, setUpdater, binding, vk::DescriptorType::eSampler} {}
auto SamplerUpdater::operator()(vk::Sampler sampler) -> void {
  ImageUpdater::operator()(sampler, {});
}

CombinedImageSamplerUpdater::CombinedImageSamplerUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : ImageUpdater{
      layoutMaker, setUpdater, binding, vk::DescriptorType::eCombinedImageSampler} {}
SampledImageUpdater::SampledImageUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding, uint32_t descriptorCount)
  : ImageUpdater{layoutMaker, setUpdater, binding, vk::DescriptorType::eSampledImage} {}
auto SampledImageUpdater::operator()(vk::ImageView imageView, vk::ImageLayout layout)
  -> void {
  ImageUpdater::operator()({}, imageView, layout);
}
StorageImageUpdater::StorageImageUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : ImageUpdater{layoutMaker, setUpdater, binding, vk::DescriptorType::eStorageImage} {}
auto StorageImageUpdater::operator()(vk::ImageView imageView) -> void {
  ImageUpdater::operator()({}, imageView, vk::ImageLayout::eGeneral);
}
InputAttachmentUpdater::InputAttachmentUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : ImageUpdater{layoutMaker, setUpdater, binding, vk::DescriptorType::eInputAttachment} {
}
auto InputAttachmentUpdater::operator()(vk::ImageView imageView) -> void {
  ImageUpdater::operator()({}, imageView);
}
UniformTexelBufferUpdater::UniformTexelBufferUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : BufferViewUpdater{
      layoutMaker, setUpdater, binding, vk::DescriptorType::eUniformTexelBuffer} {}
StorageTexelBufferUpdater::StorageTexelBufferUpdater(
  DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
  uint32_t binding)
  : BufferViewUpdater{
      layoutMaker, setUpdater, binding, vk::DescriptorType::eStorageTexelBuffer} {}
}