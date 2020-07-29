#include "frame_graph.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include <utility>

namespace vkg {

PassBuilder::PassBuilder(FrameGraphBuilder &builder, uint32_t id)
  : builder(builder), id{id} {}
auto PassBuilder::device() -> Device & { return builder.device(); }
auto PassBuilder::create(std::string_view name) -> FrameGraphResource {
  auto output = builder.create(name, id);
  outputs.push_back(output);
  return output;
}
auto PassBuilder::read(FrameGraphResource &input) -> void {
  builder.read(input, id);
  inputs.push_back(input);
}
auto PassBuilder::write(FrameGraphResource &output) -> FrameGraphResource {
  auto output_ = builder.write(output, id);
  outputs.push_back(output);
  return output_;
}
auto PassBuilder::build() -> Pass {
  Pass pass;
  pass.inputs = inputs;
  pass.outputs = outputs;
  return pass;
}

FrameGraphBuilder::FrameGraphBuilder(Device &device): device_(device) {}
auto FrameGraphBuilder::device() -> Device & { return device_; }
auto FrameGraphBuilder::addPass(PassSetup setup, PassCompile compile, PassExec execute)
  -> std::span<FrameGraphResource> {
  auto passId = uint32_t(passes.size());
  PassBuilder builder(*this, passId);
  setup(builder);
  auto pass = builder.build();
  pass.setup = std::move(setup);
  pass.compile = std::move(compile);
  pass.execute = std::move(execute);
  passes.push_back(std::move(pass));
  return passes.back().outputs;
}
auto FrameGraphBuilder::addPass(PassDef &passDef) -> std::span<FrameGraphResource> {
  auto passId = uint32_t(passes.size());
  PassBuilder builder(*this, passId);
  passDef.setup(builder);
  auto pass = builder.build();
  pass.setup = [&](PassBuilder &builder) { passDef.setup(builder); };
  pass.compile = [&](Resources &resources) { passDef.compile(resources); };
  pass.execute = [&](RenderContext &ctx, Resources &resources) {
    passDef.execute(ctx, resources);
  };
  passes.push_back(std::move(pass));
  return passes.back().outputs;
}
auto FrameGraphBuilder::create(std::string_view name, uint32_t passId)
  -> FrameGraphResource {
  errorIf(resourceIds.contains(name), "resource with name: ", name, " already exists");
  auto id_ = uint32_t(resources.size());
  resourceIds[name] = id_;
  resources.push_back({name, id_, {{passId, {}}}});
  return {name, id_, 0};
}
auto FrameGraphBuilder::read(FrameGraphResource &input, uint32_t passId) -> void {
  check(input);
  auto &revision = resources[input.id].revisions[input.revision];
  revision.readerPasses.push_back(passId);
}
auto FrameGraphBuilder::write(FrameGraphResource &output, uint32_t passId)
  -> FrameGraphResource {
  check(output);
  auto &revisions = resources[output.id].revisions;
  errorIf(
    output.revision != revisions.size() - 1,
    "write to resource should be the latest, latest revision:", revisions.size() - 1,
    ", this revision:", output.revision);
  revisions.push_back({passId});
  return {output.name, output.id, uint32_t(revisions.size() - 1)};
}
auto FrameGraphBuilder::check(FrameGraphResource &resource) -> void {
  errorIf(resource.id >= resources.size(), "resource's id is invalid: ", resource.id);
  errorIf(
    resource.revision >= resources[resource.id].revisions.size(),
    "resource's revision is invalid: ", resource.revision);
}
auto FrameGraphBuilder::createFrameGraph() -> std::unique_ptr<FrameGraph> {
  //TODO reorder passes depth first sort
  Resources res{uint32_t(resources.size())};
  return std::make_unique<FrameGraph>(device_, std::move(passes), std::move(res));
}

FrameGraph::FrameGraph(Device &device, std::vector<Pass> passes, Resources resources)
  : device_{device}, passes(std::move(passes)), resources(std::move(resources)) {}

auto FrameGraph::onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB)
  -> void {
  for(auto &pass: passes)
    pass.compile(resources);
  RenderContext ctx{device_, graphicsCB, computeCB};
  for(auto &pass: passes)
    pass.execute(ctx, resources);
}
Resources::Resources(uint32_t numResources) { physicalResources.resize(numResources); }
}
