#include "swapchain.hpp"
#include <set>
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
auto chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats)
  -> vk::SurfaceFormatKHR {
  if(formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
    return {
      VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
  for(const auto &format: formats)
    if(
      format.format == vk::Format::eB8G8R8A8Unorm &&
      format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      return format;
  return formats[0];
}

auto choosePresentMode(const std::vector<vk::PresentModeKHR> &presentModes, bool vsync)
  -> vk::PresentModeKHR {
  std::set<vk::PresentModeKHR> modes(presentModes.begin(), presentModes.end());
  auto preferredMode = vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
  return modes.contains(preferredMode) ? preferredMode : *modes.begin();
}

auto chooseExtent(
  uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR &capabilities)
  -> vk::Extent2D {
  if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.currentExtent;
  vk::Extent2D extent;
  extent.width = std::max<uint32_t>(
    capabilities.minImageExtent.width,
    std::min<uint32_t>(capabilities.maxImageExtent.width, width));
  extent.height = std::max<uint32_t>(
    capabilities.minImageExtent.height,
    std::min<uint32_t>(capabilities.maxImageExtent.height, height));
  return extent;
}

Swapchain::Swapchain(
  Device &device, vk::SurfaceKHR surface, const WindowConfig &windowConfig)
  : physicalDevice{device.physicalDevice()},
    vkDevice{device.vkDevice()},
    surface{surface},
    graphicsIndex{device.graphicsIndex()},
    presentIndex{device.presentIndex()} {

  auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  presentMode = choosePresentMode(presentModes, windowConfig.vsync);
  auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
  surfaceFormat = chooseSurfaceFormat(formats);
  presentQueue = device.presentQueue();

  auto cap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
  imageCount_ = std::clamp(2u, cap.minImageCount, cap.maxImageCount);
}
auto Swapchain::resize(uint32_t width, uint32_t height, bool vsync) -> void {
  auto cap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
  extent_ = chooseExtent(width, height, cap);

  vk::SwapchainCreateInfoKHR info;
  info.surface = surface;
  info.minImageCount = imageCount_;
  info.imageFormat = surfaceFormat.format;
  info.imageColorSpace = surfaceFormat.colorSpace;
  info.imageExtent = extent_;
  info.imageArrayLayers = 1;
  info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eStorage |
                    vk::ImageUsageFlagBits::eTransferDst;

  uint32_t queueFamilyIndices[]{graphicsIndex, presentIndex};
  if(graphicsIndex != presentIndex) {
    info.imageSharingMode = vk::SharingMode::eConcurrent;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queueFamilyIndices;
  } else
    info.imageSharingMode = vk::SharingMode::eExclusive;

  info.preTransform = cap.currentTransform;
  info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  info.presentMode = presentMode;
  info.clipped = true;
  info.oldSwapchain = *swapchain;

  swapchain = vkDevice.createSwapchainKHRUnique(info);

  images = vkDevice.getSwapchainImagesKHR(*swapchain);
  imageViews.resize(images.size());
  errorIf(
    images.size() != imageCount_,
    "newly created swapchain's imageCount != old imageCount!");
  for(auto i = 0u; i < imageCount_; i++) {
    vk::ImageViewCreateInfo imageViewInfo;
    imageViewInfo.image = images[i];
    imageViewInfo.viewType = vk::ImageViewType::e2D;
    imageViewInfo.format = surfaceFormat.format;
    imageViewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    imageViews[i] = vkDevice.createImageViewUnique(imageViewInfo);
  }
}

auto Swapchain::acquireNextImage(vk::Semaphore imageAvailable, uint32_t &imageIndex)
  -> vk::Result {
  return vkDevice.acquireNextImageKHR(
    *swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable, vk::Fence(),
    &imageIndex);
}

auto Swapchain::present(uint32_t imageIndex, vk::Semaphore renderFinished) -> vk::Result {
  vk::PresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderFinished;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &(*swapchain);
  presentInfo.pImageIndices = &imageIndex;

  return presentQueue.presentKHR(presentInfo);
}
auto Swapchain::imageCount() const -> uint32_t { return imageCount_; }
auto Swapchain::image(uint32_t index) const -> vk::Image { return images[index]; }
auto Swapchain::imageView(uint32_t index) const -> vk::ImageView {
  return *imageViews[index];
}
auto Swapchain::format() const -> vk::Format { return surfaceFormat.format; }
auto Swapchain::width() const -> uint32_t { return extent_.width; }
auto Swapchain::height() const -> uint32_t { return extent_.height; }
}