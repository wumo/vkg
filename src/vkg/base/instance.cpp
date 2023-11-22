#include "instance.hpp"
#include <string>
#include <unordered_set>
#include "window.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {

Instance::Instance(FeatureConfig featureConfig) {
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = apiVersion;

    std::unordered_set<std::string> allExtensions;
    for(auto extension: vk::enumerateInstanceExtensionProperties())
        allExtensions.insert(std::string((const char *)extension.extensionName));
    std::unordered_set<std::string> enabledExtensions;
    auto winRequired = Window::requiredExtensions();
    enabledExtensions.insert(winRequired.begin(), winRequired.end());

    std::vector<const char *> layers;
    if(allExtensions.contains(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        enabledExtensions.insert(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        supported_.deviceProperties2 = true;
    }
#if defined(USE_VALIDATION_LAYER)
    enabledExtensions.insert(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    if(featureConfig.rayTrace) {
        errorIf(
            !supported_.deviceProperties2,
            "ray tracing requires extension: ", VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        // optix requirements
        if(allExtensions.contains(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) &&
           allExtensions.contains(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME) &&
           allExtensions.contains(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)) {
            supported_.externalSync = true;
            enabledExtensions.insert(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            enabledExtensions.insert(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
            enabledExtensions.insert(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
        }
    }

    std::vector<vk::ValidationFeatureEnableEXT> enabledFeatures;
#if defined(USE_GPU_VALIDATION) || defined(USE_DEBUG_PRINTF)
    #if defined(USE_GPU_VALIDATION) && defined(USE_DEBUG_PRINTF)
    errorIf(true, "GPU Assisted Validation and Debug Printf shouldn't be enabled at the same time");
    #endif
    #if defined(USE_GPU_VALIDATION)
    enabledFeatures.push_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);
    #else
    enabledFeatures.push_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
    #endif
#endif
#if defined(USE_BEST_PRACTICES)
    enabledFeatures.push_back(vk::ValidationFeatureEnableEXT::eBestPractices);
#endif
    std::vector<const char *> extensions;
    for(const auto &extension: enabledExtensions)
        extensions.push_back(extension.c_str());
    vk::StructureChain<vk::InstanceCreateInfo, vk::ValidationFeaturesEXT> instanceInfo{
        vk::InstanceCreateInfo{
            {}, &appInfo, uint32_t(layers.size()), layers.data(), uint32_t(extensions.size()), extensions.data()},
        vk::ValidationFeaturesEXT{uint32_t(enabledFeatures.size()), enabledFeatures.data()}};

    vkInstance_ = createInstanceUnique(instanceInfo.get<vk::InstanceCreateInfo>());
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*vkInstance_);
}
auto Instance::vkInstance() -> vk::Instance { return *vkInstance_; }
auto Instance::supported() const -> const Instance::SupportedExtension & { return supported_; }

}
