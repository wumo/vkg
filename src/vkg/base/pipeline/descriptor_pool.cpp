#include "descriptor_pool.hpp"

namespace vkg {

auto DescriptorPoolMaker::pipelineLayout(const PipelineLayoutDef &def, uint32_t num) -> DescriptorPoolMaker & {
    for(auto setDef: def.layoutDef().allSetLayoutDefs())
        add(*setDef, num);
    _numSets += def.numSets() * num;
    return *this;
}
auto DescriptorPoolMaker::setLayout(const DescriptorSetDef &def, uint32_t num) -> DescriptorPoolMaker & {
    if(num == 0) return *this;
    add(def.layoutDef(), num);
    _numSets += num;
    return *this;
}

void DescriptorPoolMaker::add(const DescriptorSetLayoutMaker &setDef, uint32_t num) {
    for(auto &binding: setDef.bindings())
        switch(binding.descriptorType) {
            case vk::DescriptorType::eSampler: _numSampler += binding.descriptorCount * num; break;
            case vk::DescriptorType::eCombinedImageSampler:
                _numCombinedImageSampler += binding.descriptorCount * num;
                break;
            case vk::DescriptorType::eSampledImage: _numSampledImage += binding.descriptorCount * num; break;
            case vk::DescriptorType::eStorageImage: _numStorageImage += binding.descriptorCount * num; break;
            case vk::DescriptorType::eUniformTexelBuffer:
                _numUniformTexelBuffer += binding.descriptorCount * num;
                break;
            case vk::DescriptorType::eStorageTexelBuffer:
                _numStorageTexelBuffer += binding.descriptorCount * num;
                break;
            case vk::DescriptorType::eUniformBuffer: _numUniformBuffer += binding.descriptorCount * num; break;
            case vk::DescriptorType::eStorageBuffer: _numStorageBuffer += binding.descriptorCount * num; break;
            case vk::DescriptorType::eUniformBufferDynamic: _numUniformDynamic += binding.descriptorCount * num; break;
            case vk::DescriptorType::eStorageBufferDynamic:
                _numStorageBufferDynamic += binding.descriptorCount * num;
                break;
            case vk::DescriptorType::eInputAttachment: _numInputAttachment += binding.descriptorCount * num; break;
            case vk::DescriptorType::eInlineUniformBlockEXT:
                _numInlineUniformBlock += binding.descriptorCount * num;
                break;
            case vk::DescriptorType::eAccelerationStructureNV:
                _numAccelerationStructure += binding.descriptorCount * num;
                break;
        }
}
auto DescriptorPoolMaker::createUnique(vk::Device device) -> vk::UniqueDescriptorPool {
    std::vector<vk::DescriptorPoolSize> poolSizes{};
    if(_numSampler > 0) poolSizes.emplace_back(vk::DescriptorType::eSampler, _numSampler);
    if(_numCombinedImageSampler > 0)
        poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, _numCombinedImageSampler);
    if(_numSampledImage > 0) poolSizes.emplace_back(vk::DescriptorType::eSampledImage, _numSampledImage);
    if(_numStorageImage > 0) poolSizes.emplace_back(vk::DescriptorType::eStorageImage, _numStorageImage);
    if(_numUniformTexelBuffer > 0)
        poolSizes.emplace_back(vk::DescriptorType::eUniformTexelBuffer, _numUniformTexelBuffer);
    if(_numStorageTexelBuffer > 0)
        poolSizes.emplace_back(vk::DescriptorType::eStorageTexelBuffer, _numStorageTexelBuffer);
    if(_numUniformBuffer > 0) poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, _numUniformBuffer);
    if(_numStorageBuffer > 0) poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, _numStorageBuffer);
    if(_numUniformDynamic > 0) poolSizes.emplace_back(vk::DescriptorType::eUniformBufferDynamic, _numUniformDynamic);
    if(_numStorageBufferDynamic > 0)
        poolSizes.emplace_back(vk::DescriptorType::eStorageBufferDynamic, _numStorageBufferDynamic);
    if(_numInputAttachment > 0) poolSizes.emplace_back(vk::DescriptorType::eInputAttachment, _numInputAttachment);
    if(_numInlineUniformBlock > 0)
        poolSizes.emplace_back(vk::DescriptorType::eInlineUniformBlockEXT, _numInlineUniformBlock);
    if(_numAccelerationStructure > 0)
        poolSizes.emplace_back(vk::DescriptorType::eAccelerationStructureNV, _numAccelerationStructure);
    vk::DescriptorPoolCreateInfo descriptorPoolInfo{{}, _numSets, (uint32_t)poolSizes.size(), poolSizes.data()};
    return device.createDescriptorPoolUnique(descriptorPoolInfo);
}
}
