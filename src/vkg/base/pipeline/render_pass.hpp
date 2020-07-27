#pragma once
#include "vkg/base/vk_headers.hpp"
#include <vector>
#include <optional>

namespace vkg {
class AttachmentMaker;
class SubpassMaker;
class DependencyMaker;

class RenderPassMaker {
  friend class AttachmentMaker;
  friend class SubpassMaker;
  friend class DependencyMaker;

public:
  auto attachment(vk::Format format) -> AttachmentMaker;
  /**copy the previous defined vk::AttachmentDescription at index.*/
  auto attachmentCopy(uint32_t index) -> AttachmentMaker;
  auto attachment(const vk::AttachmentDescription &desc) -> AttachmentMaker;

  auto subpass(vk::PipelineBindPoint bindPoint, uint32_t viewmask = 0) -> SubpassMaker &;

  auto dependency(uint32_t srcSubpass, uint32_t dstSubpass) -> DependencyMaker;

  auto createUnique(vk::Device device) -> vk::UniqueRenderPass;

private:
  std::vector<vk::AttachmentDescription> attachments;
  std::vector<SubpassMaker> subpasses;
  std::vector<vk::SubpassDependency> dependencies;
  std::vector<uint32_t> multiviewMasks;
};

class AttachmentMaker {
public:
  explicit AttachmentMaker(RenderPassMaker &maker, uint32_t index);
  auto flags(vk::AttachmentDescriptionFlags value) -> AttachmentMaker &;
  auto format(vk::Format value) -> AttachmentMaker &;
  auto samples(vk::SampleCountFlagBits value) -> AttachmentMaker &;
  auto loadOp(vk::AttachmentLoadOp value) -> AttachmentMaker &;
  auto storeOp(vk::AttachmentStoreOp value) -> AttachmentMaker &;
  auto stencilLoadOp(vk::AttachmentLoadOp value) -> AttachmentMaker &;
  auto stencilStoreOp(vk::AttachmentStoreOp value) -> AttachmentMaker &;
  auto initialLayout(vk::ImageLayout value) -> AttachmentMaker &;
  auto finalLayout(vk::ImageLayout value) -> AttachmentMaker &;
  auto index() -> uint32_t;

private:
  RenderPassMaker &maker;
  uint32_t _index;
};

class SubpassMaker {
public:
  explicit SubpassMaker(vk::PipelineBindPoint bindPoint, uint32_t index);
  auto input(
    uint32_t attachment, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
    -> SubpassMaker &;
  auto color(
    uint32_t colorAttachment, uint32_t resolveAttachment = VK_ATTACHMENT_UNUSED,
    vk::ImageLayout colorLayout = vk::ImageLayout::eColorAttachmentOptimal,
    vk::ImageLayout resolveLayout = vk::ImageLayout::eColorAttachmentOptimal)
    -> SubpassMaker &;
  auto depthStencil(
    uint32_t attachment,
    vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal)
    -> SubpassMaker &;
  auto preserve(uint32_t attachment) -> SubpassMaker &;
  auto index() -> uint32_t;
  explicit operator vk::SubpassDescription() const;

private:
  const vk::PipelineBindPoint bindpoint;
  const uint32_t _index;
  std::vector<vk::AttachmentReference> inputs;
  std::vector<vk::AttachmentReference> colors;
  bool resolve{false};
  std::vector<vk::AttachmentReference> resolves;
  std::optional<vk::AttachmentReference> depth;
  std::vector<uint32_t> preserves;
};

class DependencyMaker {
public:
  explicit DependencyMaker(RenderPassMaker &renderPassMaker, uint32_t index);
  auto srcStage(vk::PipelineStageFlags value) -> DependencyMaker &;
  auto dstStage(vk::PipelineStageFlags value) -> DependencyMaker &;
  auto srcAccess(vk::AccessFlags value) -> DependencyMaker &;
  auto dstAccess(vk::AccessFlags value) -> DependencyMaker &;
  auto flags(vk::DependencyFlags value) -> DependencyMaker &;

private:
  RenderPassMaker &maker;
  uint32_t index;
};
}