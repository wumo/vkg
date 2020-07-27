#pragma once
#include "vkg/base/device.hpp"
#include <memory>

namespace vkg {
class Texture {
public:
  Texture(
    Device &device, vk::ImageCreateInfo info, VmaAllocationCreateInfo allocInfo,
    const std::string &name = "");

  template<class T = void>
  T *ptr() {
    return static_cast<T *>(alloc.pMappedData);
  }

  vk::ImageMemoryBarrier barrier(
    vk::ImageLayout newLayout, const vk::AccessFlags &dstAccess,
    vk::PipelineStageFlagBits dstStage,
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    vk::ImageAspectFlagBits aspect = vk::ImageAspectFlagBits::eColor);

  auto setImageView(vk::ImageViewType viewType, const vk::ImageAspectFlags &aspectMask)
    -> void;
  auto createLayerImageView(uint32_t layer) -> vk::UniqueImageView;
  auto recordLayout(
    vk::ImageLayout layout, vk::AccessFlags access, vk::PipelineStageFlags stage) -> void;
  auto setSampler(const vk::SamplerCreateInfo &samplerCreateInfo) -> void;

  auto device() const -> Device &;
  auto mipLevels() const -> uint32_t;
  auto arrayLayers() const -> uint32_t;
  auto image() const -> vk::Image;
  auto format() const -> vk::Format;
  auto sampler() const -> vk::Sampler;
  auto imageView() const -> vk::ImageView;
  auto extent() const -> vk::Extent3D;
  auto layout() const -> vk::ImageLayout;
  auto accessFlag() const -> vk::AccessFlags;
  auto stageFlag() const -> vk::PipelineStageFlags;
  auto aspectFlag() const -> vk::ImageAspectFlags;

private:
  struct VmaImage {
    Device &vkezDevice;
    VmaAllocation allocation{nullptr};
    vk::Image image{nullptr};
  };

  using UniquePtr = std::unique_ptr<VmaImage, std::function<void(VmaImage *)>>;
  UniquePtr vmaImage;
  VmaAllocationInfo alloc{};
  bool mappable{false};
  vk::ImageCreateInfo info;
  vk::ImageViewType viewType_;
  vk::ImageAspectFlags aspect_;

  vk::UniqueImageView imageView_;
  vk::UniqueSampler sampler_;

  vk::ImageLayout currentLayout;
  vk::AccessFlags srcAccess;
  vk::PipelineStageFlags srcStage{vk::PipelineStageFlagBits::eAllCommands};
};
}