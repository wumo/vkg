#include "pipeline_def.hpp"

namespace vkg{

auto PipelineLayoutDef::init(vk::Device device) -> void {
  pipelineLayout = layoutMaker.createUnique(device);
}
auto PipelineLayoutDef::layout() const -> vk::PipelineLayout { return *pipelineLayout; }
auto PipelineLayoutDef::numSets() const -> uint32_t { return layoutMaker.numSets(); }
auto PipelineLayoutDef::layoutDef() const -> const PipelineLayoutMaker & {
  return layoutMaker;
}
}