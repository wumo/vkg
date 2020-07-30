#pragma once
#include "pipeline_def.hpp"

namespace vkg {
#define __set__(field, def) \
public:                     \
  vkg::PipelineLayoutUpdater<def> field{layoutMaker, layoutMaker.add({})};

#define __push_constant__(field, stage, type) \
public:                                       \
  uint32_t field{layoutMaker.pushConstantAuto<type>(stage)};
}
