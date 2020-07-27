#include "device.hpp"

namespace vkg {
void Device::name(vk::Buffer object, std::string_view markerName) {
  name(uint64_t(VkBuffer(object)), vk::DebugReportObjectTypeEXT::eBuffer, markerName);
}

void Device::name(vk::Image object, std::string_view markerName) {
  name(uint64_t(VkImage(object)), vk::DebugReportObjectTypeEXT::eImage, markerName);
}

void Device::name(vk::ImageView object, std::string_view markerName) {
  name(
    uint64_t(VkImageView(object)), vk::DebugReportObjectTypeEXT::eImageView, markerName);
}

void Device::name(vk::Pipeline object, std::string_view markerName) {
  name(uint64_t(VkPipeline(object)), vk::DebugReportObjectTypeEXT::ePipeline, markerName);
}

void Device::name(vk::DescriptorSet object, std::string_view markerName) {
  name(
    uint64_t(VkDescriptorSet(object)), vk::DebugReportObjectTypeEXT::eDescriptorSet,
    markerName);
}

void Device::name(
  uint64_t object, vk::DebugReportObjectTypeEXT objectType, std::string_view markerName) {
  if(VULKAN_HPP_DEFAULT_DISPATCHER.vkDebugMarkerSetObjectNameEXT) {
    vk::DebugMarkerObjectNameInfoEXT nameInfo;
    nameInfo.object = object;
    nameInfo.objectType = objectType;
    nameInfo.pObjectName = markerName.data();

    VULKAN_HPP_DEFAULT_DISPATCHER.vkDebugMarkerSetObjectNameEXT(
      *device_, reinterpret_cast<const VkDebugMarkerObjectNameInfoEXT *>(&nameInfo));
  }
}

void Device::tag(
  uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize,
  const void *tag) {
  if(VULKAN_HPP_DEFAULT_DISPATCHER.vkDebugMarkerSetObjectTagEXT) {
    vk::DebugMarkerObjectTagInfoEXT tagInfo;
    tagInfo.object = object;
    tagInfo.objectType = objectType;
    tagInfo.tagName = name;
    tagInfo.tagSize = tagSize;
    tagInfo.pTag = tag;
    VULKAN_HPP_DEFAULT_DISPATCHER.vkDebugMarkerSetObjectTagEXT(
      *device_, reinterpret_cast<const VkDebugMarkerObjectTagInfoEXT *>(&tagInfo));
  }
}

void Device::begin(
  vk::CommandBuffer commandBuffer, std::string_view markerName,
  std::array<float, 4> color) {
  if(VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerBeginEXT) {
    vk::DebugMarkerMarkerInfoEXT markerInfo;
    memcpy(markerInfo.color, color.data(), sizeof(color));
    markerInfo.pMarkerName = markerName.data();
    VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerBeginEXT(
      commandBuffer, reinterpret_cast<const VkDebugMarkerMarkerInfoEXT *>(&markerInfo));
  }
}

void Device::insert(
  vk::CommandBuffer commandBuffer, std::string_view markerName,
  std::array<float, 4> color) {
  if(VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerInsertEXT) {
    vk::DebugMarkerMarkerInfoEXT markerInfo;
    memcpy(markerInfo.color, color.data(), sizeof(color));
    markerInfo.pMarkerName = markerName.data();
    VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerInsertEXT(
      commandBuffer, reinterpret_cast<const VkDebugMarkerMarkerInfoEXT *>(&markerInfo));
  }
}

void Device::end(vk::CommandBuffer commandBuffer) {
  if(VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerEndEXT)
    VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdDebugMarkerEndEXT(commandBuffer);
}

}