#pragma once
#include "descriptor_updaters.hpp"

namespace vkg {
class DescriptorSetDef {
public:
  auto init(vk::Device device) -> void;
  auto createSet(vk::DescriptorPool pool) -> vk::DescriptorSet;
  auto update(vk::DescriptorSet set) -> void;

  auto setLayout() -> vk::DescriptorSetLayout;
  auto layoutDef() const -> const DescriptorSetLayoutMaker &;

protected:
  DescriptorSetLayoutMaker layoutMaker{};
  DescriptorSetUpdater setUpdater{};
  vk::Device device_;
  vk::UniqueDescriptorSetLayout setLayout_{};
};
}