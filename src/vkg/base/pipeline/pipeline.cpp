#include "pipeline.hpp"

namespace vkg {

auto PipelineLayoutMaker::pushConstantOffset() const -> uint32_t {
  return pushConstantOffset_;
}
auto PipelineLayoutMaker::setPushConstantAutoOffset(uint32_t offset) -> void {
  pushConstantOffset_ = offset;
}
auto PipelineLayoutMaker::pushConstant(const vk::PushConstantRange &pushConstantRange)
  -> PipelineLayoutMaker & {
  pushConstantRanges.push_back(pushConstantRange);
  return *this;
}
auto PipelineLayoutMaker::add(vk::DescriptorSetLayout layout) -> uint32_t {
  descriptorSetLayouts.push_back(layout);
  setLayoutDefs_.push_back(nullptr);
  return uint32_t(descriptorSetLayouts.size() - 1);
}
auto PipelineLayoutMaker::update(
  uint32_t set, vk::DescriptorSetLayout layout,
  const DescriptorSetLayoutMaker &setLayoutDef) -> PipelineLayoutMaker & {
  descriptorSetLayouts.at(set) = layout;
  setLayoutDefs_.at(set) = &setLayoutDef;
  return *this;
}
auto PipelineLayoutMaker::createUnique(vk::Device device) -> vk::UniquePipelineLayout {
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount = uint32_t(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = uint32_t(pushConstantRanges.size());
  pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
  return device.createPipelineLayoutUnique(pipelineLayoutInfo);
}
auto PipelineLayoutMaker::allSetLayoutDefs() const
  -> const std::vector<const DescriptorSetLayoutMaker *> & {
  return setLayoutDefs_;
}
auto PipelineLayoutMaker::numSets() const -> uint32_t {
  return uint32_t(descriptorSetLayouts.size());
}
}