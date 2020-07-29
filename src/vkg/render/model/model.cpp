#include "model.hpp"
#include "../scene.hpp"

namespace vkg {
auto copy(Scene &scene, std::vector<uint32_t> &nodes, Node &node) -> void {
  nodes.push_back(node.id());
  for(const auto &childId: node.children()) {
    auto &child = scene.node(childId);
    copy(scene, nodes, child);
  }
}
Model::Model(
  Scene &scene, uint32_t id, const std::vector<uint32_t> &nodes,
  std::vector<Animation> &&animations)
  : id_{id}, animations_{animations} {
  for(auto nodeId: nodes) {
    auto &node = scene.node(nodeId);
    node.freeze();
  }

  for(auto nodeId: nodes) {
    auto &node = scene.node(nodeId);
    copy(scene, nodes_, node);
  }

  aabb_ = {};
  for(auto &nodeId: nodes_) {
    auto &node = scene.node(nodeId);
    aabb_.merge(node.aabb());
  }
}

auto Model::id() const -> uint32_t { return id_; }
auto Model::nodes() const -> std::span<const uint32_t> { return nodes_; }
auto Model::aabb() -> AABB { return aabb_; }
auto Model::animations() -> std::span<Animation> { return animations_; }
}
