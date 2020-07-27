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
  auto present(uint32_t imageIndex, vk::Semaphore renderFinished) -> vk::Result;

  auto imageCount() const -> uint32_t;
  auto image(uint32_t index) const -> vk::Image;
  auto imageView(uint32_t index) const -> vk::ImageView;
  auto width() const -> uint32_t;
  auto height() const -> uint32_t;
  auto format() const -> vk::Format;

private:
  vk::PhysicalDevice physicalDevice;
  vk::Device vkDevice;
  vk::SurfaceKHR surface;

  uint32_t graphicsIndex, presentIndex;

  vk::PresentModeKHR presentMode;
  vk::Queue presentQueue;
  vk::SurfaceFormatKHR surfaceFormat;

  vk::UniqueSwapchainKHR swapchain;

  uint32_t imageCount_{};
  vk::Extent2D extent_;
  std::vector<vk::Image> images;
  std::vector<vk::UniqueImageView> imageViews;
};
}