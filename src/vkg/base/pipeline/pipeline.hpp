#pragma once
#include "descriptor_def.hpp"

namespace vkg {
class PipelineLayoutMaker {
public:
  auto pushConstant(const vk::PushConstantRange &pushConstantRange)
    -> PipelineLayoutMaker &;

  template<typename T>
  auto pushConstantAuto(const vk::ShaderStageFlags &stageFlags) -> uint32_t {
    if(std::is_empty_v<T>) return pushConstantOffset_;
    auto offset = pushConstantOffset_;
    pushConstantRanges.emplace_back(stageFlags, pushConstantOffset_, sizeof(T));
    pushConstantOffset_ += sizeof(T);
    return offset;
  }

  auto pushConstantOffset() const -> uint32_t;
  auto setPushConstantAutoOffset(uint32_t offset) -> void;

  auto add(vk::DescriptorSetLayout layout) -> uint32_t;
  auto update(
    uint32_t set, vk::DescriptorSetLayout layout,
    const DescriptorSetLayoutMaker &setLayoutDef) -> PipelineLayoutMaker &;

  auto createUnique(vk::Device device) -> vk::UniquePipelineLayout;

  auto allSetLayoutDefs() const -> const std::vector<const DescriptorSetLayoutMaker *> &;

  auto numSets() const -> uint32_t;

private:
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  std::vector<vk::PushConstantRange> pushConstantRanges;
  uint32_t pushConstantOffset_{0};
  std::vector<const DescriptorSetLayoutMaker *> setLayoutDefs_;
};

template<class T>
concept DerivedDescriptorSetDef = std::is_base_of<DescriptorSetDef, T>::value;

template<DerivedDescriptorSetDef T>
class PipelineLayoutUpdater {

public:
  PipelineLayoutUpdater(PipelineLayoutMaker &layoutMaker, const uint32_t set)
    : layoutMaker(layoutMaker), set_(set) {}

  auto operator()(T &def) -> void {
    layoutMaker.update(set_, def.setLayout(), def.layoutDef());
  }

  auto set() const { return set_; }

private:
  PipelineLayoutMaker &layoutMaker;
  const uint32_t set_;
};

class SpecializationMaker {
protected:
  template<typename... Args>
  auto makeSp(Args &&... args) -> int32_t {
    std::vector<vk::SpecializationMapEntry> spentries;
    std::vector<std::byte> spdata;
    uint32_t spoffset{0};
    sp(spentries, spdata, spoffset, std::forward<Args>(args)...);
    spInfos.push_back({std::move(spentries), std::move(spdata)});
    auto &spinfo = spInfos.back();
    specializations.emplace_back(
      uint32_t(spinfo.entries.size()), spinfo.entries.data(), spinfo.data.size(),
      spinfo.data.data());
    return int32_t(specializations.size() - 1);
  }
  template<typename T, typename... Args>
  auto sp(
    std::vector<vk::SpecializationMapEntry> &spentries, std::vector<std::byte> &spdata,
    uint32_t &spoffset, T data, Args &&... rest) {
    sp(spentries, spdata, spoffset, data);
    sp(spentries, spdata, spoffset, rest...);
  }
  template<typename T>
  auto sp(
    std::vector<vk::SpecializationMapEntry> &spentries, std::vector<std::byte> &spdata,
    uint32_t &spoffset, T &&data) {
    auto size = sizeof(T);
    spentries.emplace_back(spentries.size(), spoffset, size);
    auto p = reinterpret_cast<const std::byte *>(&data);
    for(size_t i = 0; i < size; i++)
      spdata.emplace_back(*(p + i));
    spoffset += uint32_t(size);
  }
  struct spInfo {
    std::vector<vk::SpecializationMapEntry> entries;
    std::vector<std::byte> data;
  };
  std::vector<spInfo> spInfos;
  std::vector<vk::SpecializationInfo> specializations;
};
}
