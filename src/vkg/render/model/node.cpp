#include "node.hpp"
#include "vkg/render/scene.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
Node::Node(Scene &scene, uint32_t id, const Transform &transform)
  : scene{scene}, id_{id}, transform_{transform}, transf{scene.allocateTransform()} {
  *transf.ptr = transform;
}
auto Node::id() const -> uint32_t { return id_; }
auto Node::transform() const -> Transform { return transform_; }
auto Node::name() const -> std::string { return name_; }
auto Node::meshes() const -> std::span<const uint32_t> { return meshes_; }
auto Node::parent() const -> uint32_t { return parent_; }
auto Node::children() const -> std::span<const uint32_t> { return children_; }
auto Node::aabb() -> AABB { return aabb_; }
auto Node::transfOffset() const -> uint32_t { return transf.offset; }

auto Node::setTransform(const Transform &transform) -> void {
  errorIf(frozen, "Frozen node cannot be modified!");
  transform_ = transform;
  *transf.ptr = transform;
}
auto Node::setName(const std::string &name) -> void { name_ = name; }
auto Node::addMeshes(std::vector<uint32_t> &&meshes) -> void {
  errorIf(frozen, "Frozen node cannot be modified!");
  append(meshes_, meshes);
}
auto Node::addChildren(std::vector<uint32_t> &&children) -> void {
  errorIf(frozen, "Frozen node cannot be modified!");
  for(auto &childId: children) {
    auto &child = dynamic_cast<Node &>(scene.node(childId));
    errorIf(
      child.parent_ != nullIdx, "node [", childId, "] already has parent [",
      child.parent_, "]");
    errorIf(child.frozen, "cannot add frozen node as child");
    children_.emplace_back(childId);
    child.parent_ = id_;
  }
}
auto Node::freeze() -> void {
  if(frozen) return;

  // matrix update order matters, parent node should be frozen first!
  auto m = transform_.toMatrix();
  if(parent_ != nullIdx) {
    auto &parent = dynamic_cast<Node &>(scene.node(parent_));
    errorIf(!parent.frozen, "parent node should be frozen first!");
    auto parentMatrix = parent.transform_.toMatrix();
    m = parentMatrix * m;
  }
  setTransform(Transform{m});

  frozen = true;
  for(auto &childId: children_) {
    auto &child = dynamic_cast<Node &>(scene.node(childId));
    child.freeze();
  }

  // update aabb
  aabb_ = {};
  for(auto &meshId: meshes_) {
    auto &mesh = scene.mesh(meshId);
    auto primitiveId = mesh.primitive();
    auto &primitive = scene.primitive(primitiveId);
    aabb_.merge(primitive.aabb(0).transform(m)); //TODO check
  }

  for(auto &childId: children_) {
    auto &child = scene.node(childId);
    aabb_.merge(child.aabb());
  }
}
}