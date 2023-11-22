#include "render_pass.hpp"

namespace vkg {
auto RenderPassMaker::attachment(vk::Format format) -> AttachmentMaker {
    vk::AttachmentDescription desc{};
    desc.format = format;
    return attachment(desc);
}
auto RenderPassMaker::attachmentCopy(uint32_t index) -> AttachmentMaker {
    auto desc = attachments.at(index);
    return attachment(desc);
}
auto RenderPassMaker::attachment(const vk::AttachmentDescription &desc) -> AttachmentMaker {
    attachments.push_back(desc);
    return AttachmentMaker(*this, uint32_t(attachments.size() - 1));
}

auto RenderPassMaker::subpass(vk::PipelineBindPoint bindPoint, uint32_t viewmask) -> SubpassMaker & {
    subpasses.emplace_back(bindPoint, uint32_t(subpasses.size()));
    multiviewMasks.emplace_back(viewmask);
    return subpasses.back();
}

auto RenderPassMaker::dependency(uint32_t srcSubpass, uint32_t dstSubpass) -> DependencyMaker {
    vk::SubpassDependency desc;
    desc.srcSubpass = srcSubpass;
    desc.dstSubpass = dstSubpass;
    dependencies.push_back(desc);
    return DependencyMaker(*this, uint32_t(dependencies.size() - 1));
}

auto RenderPassMaker::createUnique(vk::Device device) -> vk::UniqueRenderPass {
    std::vector<vk::SubpassDescription> subpassDescs;
    for(const auto &subpass: subpasses)
        subpassDescs.push_back(vk::SubpassDescription(subpass));

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = uint32_t(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = uint32_t(subpassDescs.size());
    renderPassInfo.pSubpasses = subpassDescs.data();
    renderPassInfo.dependencyCount = uint32_t(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    bool useMultiView = false;
    for(const auto &viewmask: multiviewMasks)
        if(viewmask != 0) {
            useMultiView = true;
            break;
        }
    vk::RenderPassMultiviewCreateInfo multiviewInfo;
    multiviewInfo.subpassCount = uint32_t(multiviewMasks.size());
    multiviewInfo.pViewMasks = multiviewMasks.data();

    if(useMultiView) renderPassInfo.pNext = &multiviewInfo;

    return device.createRenderPassUnique(renderPassInfo);
}

AttachmentMaker::AttachmentMaker(RenderPassMaker &maker, uint32_t index): maker{maker}, _index{index} {}
auto AttachmentMaker::flags(vk::AttachmentDescriptionFlags value) -> AttachmentMaker & {
    maker.attachments[_index].flags = value;
    return *this;
}
auto AttachmentMaker::format(vk::Format value) -> AttachmentMaker & {
    maker.attachments[_index].format = value;
    return *this;
}
auto AttachmentMaker::samples(vk::SampleCountFlagBits value) -> AttachmentMaker & {
    maker.attachments[_index].samples = value;
    return *this;
}
auto AttachmentMaker::loadOp(vk::AttachmentLoadOp value) -> AttachmentMaker & {
    maker.attachments[_index].loadOp = value;
    return *this;
}
auto AttachmentMaker::storeOp(vk::AttachmentStoreOp value) -> AttachmentMaker & {
    maker.attachments[_index].storeOp = value;
    return *this;
}
auto AttachmentMaker::stencilLoadOp(vk::AttachmentLoadOp value) -> AttachmentMaker & {
    maker.attachments[_index].stencilLoadOp = value;
    return *this;
}
auto AttachmentMaker::stencilStoreOp(vk::AttachmentStoreOp value) -> AttachmentMaker & {
    maker.attachments[_index].stencilStoreOp = value;
    return *this;
}
auto AttachmentMaker::initialLayout(vk::ImageLayout value) -> AttachmentMaker & {
    maker.attachments[_index].initialLayout = value;
    return *this;
}
auto AttachmentMaker::finalLayout(vk::ImageLayout value) -> AttachmentMaker & {
    maker.attachments[_index].finalLayout = value;
    return *this;
}
auto AttachmentMaker::index() -> uint32_t { return _index; }

SubpassMaker::SubpassMaker(vk::PipelineBindPoint bindpoint, uint32_t index): bindpoint{bindpoint}, _index{index} {}
auto SubpassMaker::input(uint32_t attachment, vk::ImageLayout layout) -> SubpassMaker & {
    inputs.emplace_back(attachment, layout);
    return *this;
}
auto SubpassMaker::color(
    uint32_t colorAttachment, uint32_t resolveAttachment, vk::ImageLayout colorLayout, vk::ImageLayout resolveLayout)
    -> SubpassMaker & {
    colors.emplace_back(colorAttachment, colorLayout);
    resolves.emplace_back(resolveAttachment, resolveLayout);
    resolve |= resolveAttachment != VK_ATTACHMENT_UNUSED;
    return *this;
}
auto SubpassMaker::depthStencil(uint32_t attachment, vk::ImageLayout layout) -> SubpassMaker & {
    depth = {attachment, layout};
    return *this;
}
SubpassMaker &SubpassMaker::preserve(uint32_t attachment) {
    preserves.emplace_back(attachment);
    return *this;
}
auto SubpassMaker::index() -> uint32_t { return _index; }
SubpassMaker::operator vk::SubpassDescription() const {
    return vk::SubpassDescription(
        {}, bindpoint, uint32_t(inputs.size()), inputs.data(), uint32_t(colors.size()), colors.data(),
        resolve ? resolves.data() : nullptr, depth.has_value() ? &depth.value() : nullptr, uint32_t(preserves.size()),
        preserves.data());
}

DependencyMaker::DependencyMaker(RenderPassMaker &renderPassMaker, uint32_t index)
    : maker{renderPassMaker}, index{index} {}
auto DependencyMaker::srcStage(vk::PipelineStageFlags value) -> DependencyMaker & {
    maker.dependencies[index].srcStageMask = value;
    return *this;
}
auto DependencyMaker::dstStage(vk::PipelineStageFlags value) -> DependencyMaker & {
    maker.dependencies[index].dstStageMask = value;
    return *this;
}
auto DependencyMaker::srcAccess(vk::AccessFlags value) -> DependencyMaker & {
    maker.dependencies[index].srcAccessMask = value;
    return *this;
}
auto DependencyMaker::dstAccess(vk::AccessFlags value) -> DependencyMaker & {
    maker.dependencies[index].dstAccessMask = value;
    return *this;
}
auto DependencyMaker::flags(vk::DependencyFlags value) -> DependencyMaker & {
    maker.dependencies[index].dependencyFlags = value;
    return *this;
}
}