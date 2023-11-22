#pragma once
#include "vk_headers.hpp"
#include <vector>
#include <string>
#include "config.hpp"

namespace vkg {

class Instance {

public:
    struct SupportedExtension {
        bool deviceProperties2{false};
        bool externalSync{false};
    };

    explicit Instance(FeatureConfig featureConfig);

    auto vkInstance() -> vk::Instance;
    auto supported() const -> const SupportedExtension &;

private:
    uint32_t apiVersion{VK_API_VERSION_1_2};
    vk::DynamicLoader dl;
    vk::UniqueInstance vkInstance_;
    vk::UniqueDebugUtilsMessengerEXT callback;

    SupportedExtension supported_;
};
}
