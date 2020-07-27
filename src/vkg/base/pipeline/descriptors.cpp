#include "descriptors.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
auto DescriptorSetLayoutMaker::binding(
  vk::ShaderStageFlags stageFlags, uint32_t binding, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, vk::DescriptorBindingFlags bindingFlag,
  const vk::Sampler *pImmutableSamplers) -> DescriptorSetLayoutMaker & {
  _bindings.emplace_back(
    binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers);
  bindingFlags.push_back(bindingFlag);
  useDescriptorIndexing = bool(bindingFlag);
  if(bindingFlag & vk::DescriptorBindingFlagBits::eUpdateAfterBind)
    updateAfterBind = true;
  errorIf(
    variableDescriptorBinding != -1 && binding >= variableDescriptorBinding,
    "variable descriptor binding should be the largest binding!");
  if(bindingFlag & vk::DescriptorBindingFlagBits::eVariableDescriptorCount) {
    errorIf(
      variableDescriptorBinding != -1,
      "variable descriptor binding should be the largest binding!");
    variableDescriptorBinding = binding;
  }
  return *this;
}

auto DescriptorSetLayoutMaker::bindings() const
  -> const std::vector<vk::DescriptorSetLayoutBinding> & {
  return _bindings;
}
auto DescriptorSetLayoutMaker::variableDescriptorCount() -> uint32_t {
  return variableDescriptorBinding == -1 ?
           0 :
           _bindings[variableDescriptorBinding].descriptorCount;
}

auto DescriptorSetLayoutMaker::createUnique(vk::Device device)
  -> vk::UniqueDescriptorSetLayout {
  vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
  layoutCreateInfo.bindingCount = uint32_t(_bindings.size());
  layoutCreateInfo.pBindings = _bindings.data();
  vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
  if(useDescriptorIndexing) {
    bindingFlagsCreateInfo.bindingCount = uint32_t(bindingFlags.size());
    bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();
    layoutCreateInfo.pNext = &bindingFlagsCreateInfo;
  }
  if(updateAfterBind)
    layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
  return device.createDescriptorSetLayoutUnique(layoutCreateInfo);
}

auto DescriptorSetLayoutMaker::autoBinding(
  vk::ShaderStageFlags stageFlags, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, vk::DescriptorBindingFlags bindingFlag,
  const vk::Sampler *pImmutableSamplers) -> DescriptorSetLayoutMaker & {
  autoBindingIndex(
    stageFlags, descriptorType, descriptorCount, bindingFlag, pImmutableSamplers);
  return *this;
};
auto DescriptorSetLayoutMaker::autoBindingIndex(
  vk::ShaderStageFlags stageFlags, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, vk::DescriptorBindingFlags bindingFlag,
  const vk::Sampler *pImmutableSamplers) -> uint32_t {
  binding(
    stageFlags, autoBinding_, descriptorType, descriptorCount, bindingFlag,
    pImmutableSamplers);

  return autoBinding_++;
};
auto DescriptorSetLayoutMaker::setAutoBindingIndex(uint32_t binding)
  -> DescriptorSetLayoutMaker & {
  autoBinding_ = binding;
  return *this;
}

auto DescriptorSetLayoutMaker::descriptorCount(uint32_t binding) -> uint32_t & {
  return _bindings[binding].descriptorCount;
}

auto DescriptorSetMaker::layout(
  vk::DescriptorSetLayout layout, uint32_t variableDescriptorCount)
  -> DescriptorSetMaker & {
  layouts.push_back(layout);
  variableDescriptorCounts.push_back(variableDescriptorCount);
  useVariableDescriptors = useVariableDescriptors || variableDescriptorCount > 0;
  return *this;
}
auto DescriptorSetMaker::create(vk::Device device, vk::DescriptorPool descriptorPool)
  -> std::vector<vk::DescriptorSet> {
  vk::DescriptorSetAllocateInfo allocateInfo{
    descriptorPool, uint32_t(layouts.size()), layouts.data()};
  vk::DescriptorSetVariableDescriptorCountAllocateInfo info;
  if(useVariableDescriptors) {
    info.descriptorSetCount = uint32_t(variableDescriptorCounts.size());
    info.pDescriptorCounts = variableDescriptorCounts.data();
    allocateInfo.pNext = &info;
  }
  return device.allocateDescriptorSets(allocateInfo);
}

auto DescriptorSetUpdater::writeImages(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, const vk::DescriptorImageInfo *pImageInfo)
  -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{
    {}, dstBinding, dstArrayElement, descriptorCount, descriptorType, pImageInfo};
  descriptorWrites.push_back(write);
  return *this;
}
auto DescriptorSetUpdater::writeBuffers(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, const vk::DescriptorBufferInfo *pBufferInfo)
  -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{
    {},      dstBinding, dstArrayElement, descriptorCount, descriptorType,
    nullptr, pBufferInfo};
  descriptorWrites.push_back(write);
  return *this;
}
auto DescriptorSetUpdater::writeBufferViews(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  uint32_t descriptorCount, const vk::BufferView *pTexelBufferView)
  -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{
    {},      dstBinding, dstArrayElement, descriptorCount, descriptorType,
    nullptr, nullptr,    pTexelBufferView};
  descriptorWrites.push_back(write);
  return *this;
}
auto DescriptorSetUpdater::writeAccelerationStructures(
  uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount,
  const vk::AccelerationStructureKHR *pASInfo) -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{
    {},
    dstBinding,
    dstArrayElement,
    descriptorCount,
    vk::DescriptorType::eAccelerationStructureKHR};
  vk::WriteDescriptorSetAccelerationStructureKHR writeAS{descriptorCount, pASInfo};
  writeASs.push_back(writeAS);
  descriptorWrites.push_back(write);
  writeASpNextPtr.push_back({descriptorWrites.size() - 1, writeASs.size() - 1});
  return *this;
}

auto DescriptorSetUpdater::copy(
  vk::DescriptorSet srcSet, uint32_t srcBinding, uint32_t srcArrayElement,
  uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
  -> DescriptorSetUpdater & {
  descriptorCopies.emplace_back(
    srcSet, srcBinding, srcArrayElement, vk::DescriptorSet{}, dstBinding, dstArrayElement,
    descriptorCount);
  return *this;
}

auto DescriptorSetUpdater::writeImage(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  vk::DescriptorImageInfo imageInfo) -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{{}, dstBinding, dstArrayElement, 1, descriptorType};
  descriptorWrites.push_back(write);
  imageInfos.push_back(imageInfo);
  writeImageInfoPtr.push_back({descriptorWrites.size() - 1, imageInfos.size() - 1});
  return *this;
}
auto DescriptorSetUpdater::writeBuffer(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  vk::DescriptorBufferInfo bufferInfo) -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{{}, dstBinding, dstArrayElement, 1, descriptorType};
  descriptorWrites.push_back(write);
  bufferInfos.push_back(bufferInfo);
  writeBufferInfoPtr.push_back({descriptorWrites.size() - 1, bufferInfos.size() - 1});
  return *this;
}
auto DescriptorSetUpdater::writeBufferView(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType,
  vk::BufferView texelBufferView) -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{{}, dstBinding, dstArrayElement, 1, descriptorType};
  descriptorWrites.push_back(write);
  bufferViews.push_back(texelBufferView);
  writeBufferViewPtr.push_back({descriptorWrites.size() - 1, bufferViews.size() - 1});
  return *this;
}
auto DescriptorSetUpdater::writeAccelerationStructure(
  uint32_t dstBinding, uint32_t dstArrayElement, vk::AccelerationStructureKHR ASInfo)
  -> DescriptorSetUpdater & {
  vk::WriteDescriptorSet write{
    {}, dstBinding, dstArrayElement, 1, vk::DescriptorType::eAccelerationStructureKHR};
  vk::WriteDescriptorSetAccelerationStructureKHR writeAS{1};
  ASInfos.push_back(ASInfo);
  writeASs.push_back(writeAS);
  descriptorWrites.push_back(write);

  writeASpNextPtr.push_back({descriptorWrites.size() - 1, writeASs.size() - 1});
  writeASInfoPtr.push_back({descriptorWrites.size() - 1, ASInfos.size() - 1});
  return *this;
}

auto DescriptorSetUpdater::update(vk::Device device, vk::DescriptorSet dstSet) -> void {
  for(auto &write: descriptorWrites)
    write.dstSet = dstSet;
  for(auto &copy: descriptorCopies)
    copy.dstSet = dstSet;
  for(auto [writeIndex, vectorIndex]: writeImageInfoPtr)
    descriptorWrites[writeIndex].pImageInfo = imageInfos.data() + vectorIndex;
  for(auto [writeIndex, vectorIndex]: writeBufferInfoPtr)
    descriptorWrites[writeIndex].pBufferInfo = bufferInfos.data() + vectorIndex;
  for(auto [writeIndex, vectorIndex]: writeBufferViewPtr)
    descriptorWrites[writeIndex].pTexelBufferView = bufferViews.data() + vectorIndex;
  for(auto [writeIndex, vectorIndex]: writeASpNextPtr)
    descriptorWrites[writeIndex].pNext = writeASs.data() + vectorIndex;
  for(auto [writeIndex, vectorIndex]: writeASInfoPtr)
    ((vk::WriteDescriptorSetAccelerationStructureKHR *)descriptorWrites[writeIndex].pNext)
      ->pAccelerationStructures = ASInfos.data() + vectorIndex;

  device.updateDescriptorSets(descriptorWrites, descriptorCopies);

  descriptorWrites.clear();
  descriptorCopies.clear();
  writeASs.clear();
  bufferInfos.clear();
  imageInfos.clear();
  bufferViews.clear();
  ASInfos.clear();
  writeASpNextPtr.clear();
  writeImageInfoPtr.clear();
  writeBufferInfoPtr.clear();
  writeBufferViewPtr.clear();
  writeASInfoPtr.clear();
}
}