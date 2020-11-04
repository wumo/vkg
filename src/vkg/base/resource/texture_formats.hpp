#pragma once
#include "texture_formats.h"
#include "vkg/base/vk_headers.hpp"

namespace vkg::image {
auto toVulkanTextureFormat(TextureFormat format) -> vk::Format;
}