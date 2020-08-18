#include "descriptor_def.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
auto DescriptorSetDef::init(vk::Device device) -> void {
  device_ = device;
  setLayout_ = layoutMaker.createUnique(device_);
}
auto DescriptorSetDef::createSet(vk::DescriptorPool pool) -> vk::DescriptorSet {
  errorIf(!device_, "device is null, call init() first");
  errorIf(!setLayout_, "descriptorSetLayout hasn't been created!");
  return DescriptorSetMaker()
    .layout(*setLayout_, layoutMaker.variableDescriptorCount())
    .create(device_, pool)[0];
}
auto DescriptorSetDef::update(vk::DescriptorSet set) -> void {
  errorIf(!device_, "device is null, call init() first");
  setUpdater.update(device_, set);
}
auto DescriptorSetDef::setLayout() -> vk::DescriptorSetLayout { return *setLayout_; }
auto DescriptorSetDef::layoutDef() const -> const DescriptorSetLayoutMaker & {
  return layoutMaker;
}
auto DescriptorSetDef::createSets(vk::DescriptorPool pool, uint32_t num)
  -> std::vector<vk::DescriptorSet> {
  errorIf(!device_, "device is null, call init() first");
  errorIf(!setLayout_, "descriptorSetLayout hasn't been created!");
  DescriptorSetMaker maker;
  maker.layout(*setLayout_, layoutMaker.variableDescriptorCount());
  std::vector<vk::DescriptorSet> sets(num);
  for(int i = 0; i < num; ++i)
    sets[i] = maker.create(device_, pool)[0];
  return sets;
}
}