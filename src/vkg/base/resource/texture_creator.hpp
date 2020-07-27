#pragma once
#include "texture.hpp"
#include <span>

namespace vkg::image {
uint32_t mipLevels(uint32_t dim);

/**
 *
 * @param format default is vk::Format::eR8G8B8A8Unorm, do `sRGB` conversion in shader.
 * @return
 */
auto make2DTex(
  Device &device, uint32_t width, uint32_t height, vk::ImageUsageFlags usage,
  vk::Format format = vk::Format::eR8G8B8A8Unorm,
  vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1,
  vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, bool useMipmap = false)
  -> std::unique_ptr<Texture>;

auto make2DArrayTex(
  Device &device, uint32_t width, uint32_t height, uint32_t arrayLayers,
  vk::ImageUsageFlags usage, vk::Format format, vk::SampleCountFlagBits sampleCount,
  vk::ImageAspectFlags aspect, bool useMipmap = false) -> std::unique_ptr<Texture>;

auto makeSampler2DTex(
  Device &device, uint32_t width, uint32_t height,
  vk::Format format = vk::Format::eR8G8B8A8Unorm, bool useMipmap = false)
  -> std::unique_ptr<Texture>;

auto makeDepthStencilTex(
  Device &device, uint32_t width, uint32_t height,
  vk::Format format = vk::Format::eD24UnormS8Uint,
  vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
  -> std::unique_ptr<Texture>;

auto makeColorInputAtt(
  Device &device, uint32_t width, uint32_t height,
  vk::Format format = vk::Format::eR8G8B8A8Unorm,
  vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
  -> std::unique_ptr<Texture>;

auto makeStorageAtt(
  Device &device, uint32_t width, uint32_t height, vk::Format format,
  vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
  -> std::unique_ptr<Texture>;

auto makeLinearHostTex(Device &device, uint32_t width, uint32_t height, vk::Format format)
  -> std::unique_ptr<Texture>;

auto makeDepthStencilInputAtt(
  Device &device, uint32_t width, uint32_t height,
  vk::Format format = vk::Format::eD24UnormS8Uint,
  vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1)
  -> std::unique_ptr<Texture>;

auto load2DFromFile(
  Device &device, const std::string &file, bool mipmap = false,
  vk::Format format = vk::Format::eR8G8B8A8Unorm) -> std::unique_ptr<Texture>;

auto load2DFromGrayScaleFile(
  Device &device, const std::string &file, bool mipmap = false,
  vk::Format format = vk::Format::eR16Sfloat) -> std::unique_ptr<Texture>;

auto load2DFromBytes(
  Device &device, std::span<std::byte> bytes, uint32_t texWidth, uint32_t texHeight,
  bool mipmap = false, vk::Format format = vk::Format::eR8G8B8A8Unorm)
  -> std::unique_ptr<Texture>;
}
