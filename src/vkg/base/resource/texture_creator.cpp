#include "texture_creator.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include "texture_upload.hpp"
#include "texture_mipmap.hpp"
#include <stb_image.h>

namespace vkg::image {
uint32_t mipLevels(uint32_t dim) { return static_cast<uint32_t>(std::floor(std::log2(dim))) + 1; }

auto make2DTex(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::ImageUsageFlags usage,
    vk::Format format, vk::SampleCountFlagBits sampleCount, vk::ImageAspectFlags aspect, bool useMipmap)
    -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            useMipmap ? mipLevels(std::max(width, height)) : 1,
            1,
            sampleCount,
            vk::ImageTiling::eOptimal,
            usage},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, aspect);
    return texture;
}

auto make2DArrayTex(
    const std::string &name, Device &device, uint32_t width, uint32_t height, uint32_t arrayLayers,
    vk::ImageUsageFlags usage, vk::Format format, vk::SampleCountFlagBits sampleCount, vk::ImageAspectFlags aspect,
    bool useMipmap) -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            useMipmap ? mipLevels(std::max(width, height)) : 1,
            arrayLayers,
            sampleCount,
            vk::ImageTiling::eOptimal,
            usage},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2DArray, aspect);
    return texture;
}

auto makeSampler2DTex(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format, bool useMipmap)
    -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            useMipmap ? mipLevels(std::max(width, height)) : 1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                vk::ImageUsageFlagBits::eTransferDst},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    return texture;
}

auto makeDepthStencilTex(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format,
    vk::SampleCountFlagBits sampleCount) -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            1,
            1,
            sampleCount,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);
    return texture;
}

auto makeColorInputAtt(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format,
    vk::SampleCountFlagBits sampleCount) -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            1,
            1,
            sampleCount,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    return texture;
}

auto makeStorageAtt(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format,
    vk::SampleCountFlagBits sampleCount) -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1},
            1,
            1,
            sampleCount,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled |
                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    return texture;
}

auto makeLinearHostTex(const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format)
    -> std::unique_ptr<Texture> {
    return std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1},
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eLinear,
            vk::ImageUsageFlagBits::eTransferDst},
        VmaAllocationCreateInfo{
            VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        name);
}

auto makeDepthStencilInputAtt(
    const std::string &name, Device &device, uint32_t width, uint32_t height, vk::Format format,
    vk::SampleCountFlagBits sampleCount) -> std::unique_ptr<Texture> {
    auto texture = std::make_unique<Texture>(
        device,
        vk::ImageCreateInfo{
            {},
            vk::ImageType::e2D,
            format,
            {width, height, 1U},
            1,
            1,
            sampleCount,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment},
        VmaAllocationCreateInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY}, name);
    texture->setImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);
    return texture;
}

auto load2DFromFile(
    uint32_t queueIdx, const std::string &name, Device &device, const std::string &file, bool mipmap, vk::Format format)
    -> std::unique_ptr<Texture> {
    uint32_t texWidth, texHeight, texChannels;
    auto pixels = UniqueBytes(
        stbi_load(
            file.c_str(), reinterpret_cast<int *>(&texWidth), reinterpret_cast<int *>(&texHeight),
            reinterpret_cast<int *>(&texChannels), STBI_rgb_alpha),
        [](stbi_uc *ptr) { stbi_image_free(ptr); });

    errorIf(pixels == nullptr, "failed to load texture ", file);
    auto texture = makeSampler2DTex(name, device, texWidth, texHeight, format, mipmap);
    auto imageSize = texWidth * texHeight * STBI_rgb_alpha * sizeof(stbi_uc);
    upload(queueIdx, *texture, {(std::byte *)(pixels.get()), imageSize}, !mipmap);
    if(mipmap) generateMipmap(queueIdx, *texture);
    return texture;
}

auto load2DFromMemory(
    uint32_t queueIdx, const std::string &name, Device &device, std::span<std::byte> bytes, bool mipmap,
    vk::Format format) -> std::unique_ptr<Texture> {
    uint32_t texWidth, texHeight, texChannels;
    auto pixels = UniqueBytes(
        stbi_load_from_memory(
            reinterpret_cast<const stbi_uc *>(bytes.data()), bytes.size_bytes(), reinterpret_cast<int *>(&texWidth),
            reinterpret_cast<int *>(&texHeight), reinterpret_cast<int *>(&texChannels), STBI_rgb_alpha),
        [](stbi_uc *ptr) { stbi_image_free(ptr); });

    errorIf(pixels == nullptr, "failed to load texture ", name);
    auto texture = makeSampler2DTex(name, device, texWidth, texHeight, format, mipmap);
    auto imageSize = texWidth * texHeight * STBI_rgb_alpha * sizeof(stbi_uc);
    upload(queueIdx, *texture, {(std::byte *)(pixels.get()), imageSize}, !mipmap);
    if(mipmap) generateMipmap(queueIdx, *texture);
    return texture;
}

auto load2DFromGrayScaleFile(
    uint32_t queueIdx, const std::string &name, Device &device, const std::string &file, bool mipmap, vk::Format format)
    -> std::unique_ptr<Texture> {
    return std::unique_ptr<Texture>();
}

auto load2DFromBytes(
    uint32_t queueIdx, const std::string &name, Device &device, std::span<std::byte> bytes, uint32_t texWidth,
    uint32_t texHeight, bool mipmap, vk::Format format) -> std::unique_ptr<Texture> {
    auto texture = makeSampler2DTex(name, device, texWidth, texHeight, format, mipmap);
    upload(queueIdx, *texture, bytes, !mipmap);
    //  upload(*texture, pixels.get(), imageSize, !mipmap);
    if(mipmap) generateMipmap(queueIdx, *texture);
    return texture;
}
}
