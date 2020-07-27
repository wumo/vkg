#pragma once
#include "descriptors.hpp"
#include "vkg/base/resource/textures.hpp"

namespace vkg {
class DescriptorUpdater {
public:
  DescriptorUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);

  const uint32_t binding;

  auto descriptorCount() -> uint32_t &;

protected:
  DescriptorSetLayoutMaker &layoutMaker;
  DescriptorSetUpdater &setUpdater;
};

class BufferUpdater: public DescriptorUpdater {
public:
  BufferUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding, vk::DescriptorType descriptorType);

  auto operator()(
    uint32_t dstArrayElement, uint32_t descriptorCount,
    const vk::DescriptorBufferInfo *pBufferInfo) -> void;
  auto operator()(
    uint32_t dstArrayElement, vk::Buffer buffer, vk::DeviceSize offset = 0,
    vk::DeviceSize range = VK_WHOLE_SIZE) -> void;
  auto operator()(
    vk::Buffer buffer, vk::DeviceSize offset = 0, vk::DeviceSize range = VK_WHOLE_SIZE)
    -> void;

private:
  vk::DescriptorType descriptorType;
};

class ImageUpdater: public DescriptorUpdater {
public:
  ImageUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding, vk::DescriptorType descriptorType);

  auto operator()(
    uint32_t dstArrayElement, uint32_t descriptorCount,
    const vk::DescriptorImageInfo *pImageInfo) -> void;
  auto operator()(
    vk::Sampler sampler, vk::ImageView imageView,
    vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) -> void;
  auto operator()(
    Texture &texture, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
    -> void;

private:
  vk::DescriptorType descriptorType;
};

class BufferViewUpdater: public DescriptorUpdater {
public:
  BufferViewUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding, vk::DescriptorType descriptorType);

  auto operator()(
    uint32_t dstArrayElement, uint32_t descriptorCount,
    const vk::BufferView *pTexelBufferView) -> void;
  auto operator()(vk::BufferView texelBufferView) -> void;

private:
  vk::DescriptorType descriptorType;
};

class AccelerationStructureUpdater: public DescriptorUpdater {
public:
  AccelerationStructureUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);

  auto operator()(
    uint32_t dstArrayElement, uint32_t descriptorCount,
    const vk::AccelerationStructureNV *pASInfo) -> void;
  auto operator()(vk::AccelerationStructureNV ASInfo) -> void;
};

class UniformUpdater: public BufferUpdater {
public:
  UniformUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};
class UniformDynamicUpdater: public BufferUpdater {
public:
  UniformDynamicUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};
class StorageBufferUpdater: public BufferUpdater {
public:
  StorageBufferUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};
class StorageBufferDynamicUpdater: public BufferUpdater {
public:
  StorageBufferDynamicUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding, uint32_t descriptorCount);
};

class SamplerUpdater: public ImageUpdater {
public:
  SamplerUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
  auto operator()(vk::Sampler sampler) -> void;
};
class CombinedImageSamplerUpdater: public ImageUpdater {
public:
  CombinedImageSamplerUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};
class SampledImageUpdater: public ImageUpdater {
public:
  SampledImageUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding, uint32_t descriptorCount);
  auto operator()(
    vk::ImageView imageView,
    vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) -> void;
};
class StorageImageUpdater: public ImageUpdater {
public:
  StorageImageUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
  auto operator()(vk::ImageView imageView) -> void;
};
class InputAttachmentUpdater: public ImageUpdater {
public:
  InputAttachmentUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
  auto operator()(vk::ImageView imageView) -> void;
};

class UniformTexelBufferUpdater: public BufferViewUpdater {
public:
  UniformTexelBufferUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};
class StorageTexelBufferUpdater: public BufferViewUpdater {
public:
  StorageTexelBufferUpdater(
    DescriptorSetLayoutMaker &layoutMaker, DescriptorSetUpdater &setUpdater,
    uint32_t binding);
};

}