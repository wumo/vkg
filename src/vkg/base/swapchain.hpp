#pragma once
#include <cstdint>
#include <vector>
#include "vk_headers.hpp"
#include "device.hpp"

namespace vkg {
class Swapchain {
public:
  Swapchain(Device &device, vk::SurfaceKHR surface, const WindowConfig &windowConfig);
  auto resize(uint32_t width, uint32_t height, bool vsync) -> void;

  auto acquireNextImage(vk::Semaphore imageAvailable, uint32_t &imageIndex) -> vk::Result;
  auto present(uint32_t frameIdx, uint32_t imageIdx, vk::Semaphore renderFinished)
    -> vk::Result;

  auto imageCount() const -> uint32_t;
  auto image(uint32_t index) const -> vk::Image;
  auto imageView(uint32_t index) const -> vk::ImageView;
  auto imageExtent() const -> vk::Extent2D;
  auto width() const -> uint32_t;
  auto height() const -> uint32_t;
  auto format() const -> vk::Format;
  auto version() const -> uint64_t;

private:
  Device &device;
  vk::PhysicalDevice physicalDevice;
  vk::Device vkDevice;
  vk::SurfaceKHR surface;

  std::span<vk::Queue> queues;

  vk::PresentModeKHR presentMode;
  vk::SurfaceFormatKHR surfaceFormat;

  vk::UniqueSwapchainKHR swapchain;
  
  vk::Extent2D extent_;
  uint32_t imageCount_;
  std::vector<vk::Image> images;
  std::vector<vk::UniqueImageView> imageViews;

  uint64_t version_{0};
};
}
