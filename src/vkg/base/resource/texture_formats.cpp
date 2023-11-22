#include "texture_formats.hpp"

namespace vkg::image {

auto toVulkanTextureFormat(TextureFormat format) -> vk::Format {
    switch(format) {
        case R8Unorm: return vk::Format::eR8Unorm;
        case R16Sfloat: return vk::Format::eR16Sfloat;
        case R32Sfloat: return vk::Format::eR32Sfloat;
        case R8G8B8A8Unorm: return vk::Format::eR8G8B8A8Unorm;
        case R16G16B16A16Sfloat: return vk::Format::eR16G16B16A16Sfloat;
        case R32G32B32A32Sfloat: return vk::Format::eR32G32B32A32Sfloat;
    }
}
}