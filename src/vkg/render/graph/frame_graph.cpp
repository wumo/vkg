#include "frame_graph.hpp"

namespace vkg {

PassBuilder::PassBuilder(FrameGraph &frameGraph, uint32_t id, BasePass &pass)
  : frameGraph(frameGraph), id{id}, pass_{pass} {}
auto PassBuilder::device() -> Device & { return frameGraph.device(); }
auto PassBuilder::nameMangling(std::string name) -> std::string {
  return pass_.name + "/" + name;
}

Resources::Resources(Device &device, uint32_t numResources): device{device} {
  physicalResources.resize(numResources);
}

FrameGraph::FrameGraph(Device &device): device_(device) {}
auto FrameGraph::device() -> Device & { return device_; }
auto FrameGraph::check(FrameGraphBaseResource &resource) -> void {
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
    for(auto &output: passes[u]->outputs_) {
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
    if(visited[i] == State::eUnVisited && passes[i]->inputs_.empty()) dfs(i);

  for(auto i = 0u; i < n; ++i) {
    auto passId = sortedPassIds[i];
    auto &pass = passes[passId];
    pass->order = i;
  }

  for(auto i = 0u; i < n; ++i) {
    auto passId = sortedPassIds[i];
    auto &pass = passes[passId];
    std::map<uint32_t, std::vector<FrameGraphBaseResource>> nextPasses;
    for(auto &output: pass->outputs_)
      for(auto &p: resRevisions[output.id].revisions[output.revision].readerPasses)
        nextPasses[p].push_back(output);

    for(auto &[nextPassId, res]: nextPasses) {
      auto &nextPass = passes[nextPassId];
      print("[", pass->name, "$", passId, ":", pass->order, "] -> [");
      println(nextPass->name, "$", nextPassId, ":", nextPass->order, "]:");
      for(auto &r: res)
        println("\t(", resRevisions[r.id].name, "$", r.id, ":", r.revision, ")");
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
