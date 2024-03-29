#pragma once
#include "descriptor_def.hpp"

namespace vkg {
#define __uniform__(field, stage)                                                                        \
public:                                                                                                  \
    vkg::UniformUpdater field {                                                                          \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eUniformBuffer) \
    }
#define __uniformDynamic__(field, stage)                                                                        \
public:                                                                                                         \
    vkg::UniformDynamicUpdater field {                                                                          \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eUniformBufferDynamic) \
    }
#define __buffer__(field, stage)                                                                         \
public:                                                                                                  \
    vkg::StorageBufferUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageBuffer) \
    }
#define __bufferDynamic__(field, stage)                                                                         \
public:                                                                                                         \
    vkg::StorageBufferDynamicUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageBufferDynamic) \
    }
#define __accelerationStructure__(field, stage)                                                                    \
public:                                                                                                            \
    vkg::AccelerationStructureUpdater field {                                                                      \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eAccelerationStructureNV) \
    }
#define __sampler__(field, stage)                                                                  \
public:                                                                                            \
    vkg::SamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampler) \
    }

#define __image1D__(field, stage)                                                                       \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __image1DArray__(field, stage)                                                                  \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __image2D__(field, stage)                                                                       \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __image2DRect__(field, stage)                                                                   \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __image2DArray__(field, stage)                                                                  \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __image3D__(field, stage)                                                                       \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __imageCube__(field, stage)                                                                     \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }
#define __imageCubeArray__(field, stage)                                                                \
public:                                                                                                 \
    vkg::StorageImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageImage) \
    }

#define __texture1D__(field, stage)                                                                     \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __texture1DArray__(field, stage)                                                                \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __texture2D__(field, stage)                                                                     \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __texture2DArray__(field, stage)                                                                \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __texture3D__(field, stage)                                                                     \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __textureCube__(field, stage)                                                                   \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }
#define __textureCubeArray__(field, stage)                                                              \
public:                                                                                                 \
    vkg::SampledImageUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eSampledImage) \
    }

#define __sampler1D__(field, stage)                                                                             \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __sampler1DArray__(field, stage)                                                                        \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __sampler2D__(field, stage)                                                                             \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __sampler2DArray__(field, stage)                                                                        \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __sampler3D__(field, stage)                                                                             \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __samplerCube__(field, stage)                                                                           \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }
#define __samplerCubeArray__(field, stage)                                                                      \
public:                                                                                                         \
    vkg::CombinedImageSamplerUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eCombinedImageSampler) \
    }

#define __input__(field, stage)                                                                            \
public:                                                                                                    \
    vkg::InputAttachmentUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eInputAttachment) \
    }

#define __textureBuffer__(field, stage)                                                                       \
public:                                                                                                       \
    vkg::UniformTexelBufferUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eUniformTexelBuffer) \
    }
#define __imageBuffer__(field, stage)                                                                         \
public:                                                                                                       \
    vkg::StorageTexelBufferUpdater field {                                                                    \
        layoutMaker, setUpdater, layoutMaker.autoBindingIndex(stage, vk::DescriptorType::eStorageTexelBuffer) \
    }
}