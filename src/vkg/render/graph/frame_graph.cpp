#include "frame_graph.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include <utility>

namespace vkg {

LambdaPass::LambdaPass(PassSetup setup, PassCompile compile, PassExec execute)
  : setup_(std::move(setup)),
    compile_(std::move(compile)),
    execute_(std::move(execute)) {}
void LambdaPass::setup(PassBuilder &builder) { setup_(builder); }
void LambdaPass::compile(Resources &resources) { compile_(resources); }
void LambdaPass::execute(RenderContext &ctx, Resources &resources) {
  execute_(ctx, resources);
}

PassBuilder::PassBuilder(FrameGraph &frameGraph, uint32_t id, Pass &pass)
  : frameGraph(frameGraph), id{id}, pass{pass} {}
auto PassBuilder::device() -> Device & { return frameGraph.device(); }
auto PassBuilder::create(std::string name, ResourceType type) -> FrameGraphResource {
  auto output = frameGraph.create(std::move(name), id, type);
  outputs.push_back(output);
  return output;
}
auto PassBuilder::read(FrameGraphResource &input) -> void {
  frameGraph.read(input, id);
  errorIf(
    uniqueInputs.contains(input.id), "already read resource ", input.id, " as input");
  uniqueInputs.insert(input.id);
  inputs.push_back(input);
}
auto PassBuilder::write(FrameGraphResource &input) -> FrameGraphResource {
  auto output = frameGraph.write(input, id);
  errorIf(
    uniqueInputs.contains(input.id), "already read resource ", input.id, " as input");
  uniqueInputs.insert(input.id);
  inputs.push_back(input);
  outputs.push_back(output);
  return output;
}
auto PassBuilder::build() -> void {
  pass.setup(*this);
  pass.inputs = std::move(inputs);
  pass.outputs = std::move(outputs);
}

Resources::Resources(Device &device, uint32_t numResources): device{device} {
  physicalResources.resize(numResources);
}

FrameGraph::FrameGraph(Device &device): device_(device) {}
auto FrameGraph::device() -> Device & { return device_; }
auto FrameGraph::addPass(
  std::string name, PassSetup setup, PassCompile compile, PassExec execute)
  -> std::span<FrameGraphResource> {
  return addPass<LambdaPass>(
    std::move(name), std::move(setup), std::move(compile), std::move(execute));
}
auto FrameGraph::addPass(std::string name, Pass &pass) -> std::span<FrameGraphResource> {
  errorIf(frozen, "frame graph already frozen");
  auto passId = uint32_t(passes.size());
  pass.name = std::move(name);
  pass.id = passId;
  passes.push_back(&pass);
  PassBuilder builder(*this, passId, pass);
  builder.build();
  return pass.outputs;
}
auto FrameGraph::create(const std::string &name, uint32_t passId, ResourceType type)
  -> FrameGraphResource {
  errorIf(resourceIds.contains(name), "resource with name: ", name, " already exists");
  auto id_ = uint32_t(resRevisions.size());
  resourceIds[name] = id_;
  resRevisions.push_back({name, id_, type, {{passId, {}}}});
  return {name, id_, 0, type};
}
auto FrameGraph::read(FrameGraphResource &input, uint32_t passId) -> void {
  check(input);
  auto &revision = resRevisions[input.id].revisions[input.revision];
  revision.readerPasses.push_back(passId);
}
auto FrameGraph::write(FrameGraphResource &input, uint32_t passId) -> FrameGraphResource {
  check(input);
  auto &revisions = resRevisions[input.id].revisions;
  errorIf(
    input.revision != revisions.size() - 1,
    "write to resource should be the latest, latest revision:", revisions.size() - 1,
    ", this revision:", input.revision);
  revisions.push_back({passId});
  return {input.name, input.id, uint32_t(revisions.size() - 1)};
}
auto FrameGraph::check(FrameGraphResource &resource) -> void {
  errorIf(resource.id >= resRevisions.size(), "resource's id is invalid: ", resource.id);
  errorIf(
    resource.revision >= resRevisions[resource.id].revisions.size(),
    "resource's revision is invalid: ", resource.revision);
}

auto FrameGraph::build() -> void {
  errorIf(frozen, "frame graph already frozen");
  frozen = true;
  //topological sort
  const auto n = uint32_t(passes.size());
  if(n == 0) return;

  enum class State : uint32_t { eUnVisited = 0, eBeginVisit = 1, eFinishVisit = 2 };
  std::vector<State> visited;
  visited.resize(n);
  sortedPassIds.resize(n);

  uint32_t orderIdx = n - 1;

  std::function<void(uint32_t)> dfs;
  dfs = [&](uint32_t u) {
    visited[u] = State::eBeginVisit;
    for(auto &output: passes[u]->outputs) {
      auto &revision = resRevisions[output.id].revisions[output.revision];
      errorIf(
        revision.writerPass != u,
        "resource revision's writer pass is not consistent with pass's output");
      for(auto &v: revision.readerPasses)
        switch(visited[v]) {
          case State::eUnVisited: dfs(v); break;
          case State::eBeginVisit: error("cycles in frame graph");
          case State::eFinishVisit: continue;
        }
    }
    sortedPassIds[orderIdx--] = u;
    visited[u] = State::eFinishVisit;
  };

  for(auto i = 0u; i < n; ++i)
    if(visited[i] == State::eUnVisited && passes[i]->inputs.empty()) dfs(i);

  for(int i = 0; i < n; ++i) {
    auto passId = sortedPassIds[i];
    auto &pass = passes[passId];
    pass->order = i;
  }

  for(int i = 0; i < n; ++i) {
    auto passId = sortedPassIds[i];
    auto &pass = passes[passId];
    std::map<uint32_t, std::vector<FrameGraphResource>> nextPasses;
    for(auto &output: pass->outputs)
      for(auto &p: resRevisions[output.id].revisions[output.revision].readerPasses)
        nextPasses[p].push_back(output);

    for(auto &[nextPassId, res]: nextPasses) {
      auto &nextPass = passes[nextPassId];
      print("[", pass->name, "-", passId, ":", pass->order, "] -> [");
      println(nextPass->name, "-", nextPassId, ":", nextPass->order, "]:");
      for(auto &r: res)
        println("\t", r.name, "-", r.id, ":", r.revision);
    }
  }

  resources = std::make_unique<Resources>(device_, uint32_t(resRevisions.size()));
}

auto FrameGraph::onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB)
  -> void {
  for(auto &id: sortedPassIds)
    passes[id]->compile(*resources);
  RenderContext ctx{device_, graphicsCB, computeCB};
  for(auto &id: sortedPassIds)
    passes[id]->execute(ctx, *resources);
}
}
