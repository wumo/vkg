#pragma once
#include "vkg/base/vk_headers.hpp"
#include <vector>

namespace vkg {
class DescriptorSetLayoutMaker {
public:
  auto binding(
    vk::ShaderStageFlags stageFlags, uint32_t binding, vk::DescriptorType descriptorType,
    uint32_t descriptorCount = 1, vk::DescriptorBindingFlags bindingFlag = {},
    const vk::Sampler *pImmutableSamplers = nullptr) -> DescriptorSetLayoutMaker &;

  auto bindings() const -> const std::vector<vk::DescriptorSetLayoutBinding> &;
  auto variableDescriptorCount() -> uint32_t;

  auto createUnique(vk::Device device) -> vk::UniqueDescriptorSetLayout;

  auto autoBinding(
    vk::ShaderStageFlags stageFlags, vk::DescriptorType descriptorType,
    uint32_t descriptorCount = 1, vk::DescriptorBindingFlags bindingFlag = {},
    const vk::Sampler *pImmutableSamplers = nullptr) -> DescriptorSetLayoutMaker &;
  auto autoBindingIndex(
    vk::ShaderStageFlags stageFlags, vk::DescriptorType descriptorType,
    uint32_t descriptorCount = 1, vk::DescriptorBindingFlags bindingFlag = {},
    const vk::Sampler *pImmutableSamplers = nullptr) -> uint32_t;
  auto setAutoBindingIndex(uint32_t binding) -> DescriptorSetLayoutMaker &;

  auto descriptorCount(uint32_t binding) -> uint32_t &;

private:
  std::vector<vk::DescriptorSetLayoutBinding> _bindings;
  std::vector<vk::DescriptorBindingFlags> bindingFlags;
  bool useDescriptorIndexing{false};
  bool updateAfterBind{false};
  int variableDescriptorBinding{-1};
  uint32_t autoBinding_{0};
};

class DescriptorSetMaker {
public:
  /**
   * @param variableDescriptorCount the descriptor count of the variable count descriptor.
   */
  auto layout(vk::DescriptorSetLayout layout, uint32_t variableDescriptorCount = 0)
    -> DescriptorSetMaker &;
  auto create(vk::Device device, vk::DescriptorPool descriptorPool)
    -> std::vector<vk::DescriptorSet>;

private:
  std::vector<vk::DescriptorSetLayout> layouts;
  std::vector<uint32_t> variableDescriptorCounts;
  bool useVariableDescriptors{false};
};

class DescriptorSetUpdater {
public:
  auto writeImages(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    uint32_t descriptorCount, const vk::DescriptorImageInfo *pImageInfo)
    -> DescriptorSetUpdater &;
  auto writeBuffers(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    uint32_t descriptorCount, const vk::DescriptorBufferInfo *pBufferInfo)
    -> DescriptorSetUpdater &;
  auto writeBufferViews(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    uint32_t descriptorCount, const vk::BufferView *pTexelBufferView)
    -> DescriptorSetUpdater &;
  auto writeAccelerationStructures(
    uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount,
    const vk::AccelerationStructureNV *pASInfo) -> DescriptorSetUpdater &;

  auto writeImage(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    vk::DescriptorImageInfo imageInfo) -> DescriptorSetUpdater &;
  auto writeBuffer(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    vk::DescriptorBufferInfo bufferInfo) -> DescriptorSetUpdater &;
  auto writeBufferView(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
    vk::BufferView texelBufferView) -> DescriptorSetUpdater &;
  auto writeAccelerationStructure(
    uint32_t dstBinding, uint32_t dstArrayElement, vk::AccelerationStructureNV ASInfo)
    -> DescriptorSetUpdater &;

  auto copy(
    vk::DescriptorSet srcSet, uint32_t srcBinding, uint32_t srcArrayElement,
    uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
    -> DescriptorSetUpdater &;

  auto update(vk::Device device, vk::DescriptorSet dstSet) -> void;

private:
  std::vector<vk::WriteDescriptorSet> descriptorWrites;
  std::vector<vk::CopyDescriptorSet> descriptorCopies;

  struct Index {
    size_t writeIndex;
    size_t vectorIndex;
  };

  std::vector<Index> writeASpNextPtr;

  std::vector<Index> writeImageInfoPtr;

  std::vector<Index> writeBufferInfoPtr;

  std::vector<Index> writeBufferViewPtr;

  std::vector<Index> writeASInfoPtr;

  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  std::vector<vk::DescriptorImageInfo> imageInfos;
  std::vector<vk::BufferView> bufferViews;
  std::vector<vk::WriteDescriptorSetAccelerationStructureNV> writeASs;
  std::vector<vk::AccelerationStructureNV> ASInfos;
};
}
