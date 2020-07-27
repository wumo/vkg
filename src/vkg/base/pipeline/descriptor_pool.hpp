#pragma once
#include "pipeline_def.hpp"
#include "descriptor_def.hpp"

namespace vkg {
class DescriptorPoolMaker {
public:
  auto pipelineLayout(const PipelineLayoutDef &def, uint32_t num = 1) -> DescriptorPoolMaker &;
  auto setLayout(const DescriptorSetDef &def, uint32_t num = 1) -> DescriptorPoolMaker &;
  auto createUnique(vk::Device device) -> vk::UniqueDescriptorPool;

private:
  void add(const DescriptorSetLayoutMaker &setDef, uint32_t num);

  uint32_t _numSampler{0}, _numCombinedImageSampler{0}, _numSampledImage{0},
    _numStorageImage{0}, _numUniformTexelBuffer{0}, _numStorageTexelBuffer{0},
    _numUniformBuffer{0}, _numStorageBuffer{0}, _numUniformDynamic{0},
    _numStorageBufferDynamic{0}, _numInputAttachment{0}, _numInlineUniformBlock{0},
    _numAccelerationStructure{0};
  uint32_t _numSets{0};
};
}