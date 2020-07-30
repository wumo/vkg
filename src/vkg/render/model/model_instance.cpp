#include "model_instance.hpp"
#include "vkg/render/scene.hpp"
namespace vkg {

ModelInstance::ModelInstance(
  Scene &scene, uint32_t id, const Transform &transform, uint32_t modelId)
  : scene{scene},
    id_{id},
    transform_{transform},
    model_{modelId},
    transf{scene.allocateTransform()} {
  *transf.ptr = transform;

  for(auto &m = scene.model(modelId); const auto &nodeId: m.nodes())
    for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
      if(meshId == nullIdx) continue;
      auto &mesh = scene.mesh(meshId);
      auto &primitive = scene.primitive(mesh.primitive());
      auto &material = scene.material(mesh.material());

      auto meshInsDesc = scene.allocateMeshInstDesc();
      *meshInsDesc.ptr = {
        material.descOffset(),
        primitive.descOffset(),
        node.transfOffset(),
        transf.offset,
        true,
        scene.addToDrawGroup(meshId)};
      meshInstDescs.push_back(meshInsDesc);
    }
}
auto ModelInstance::id() const -> uint32_t { return id_; }
auto ModelInstance::transform() const -> Transform { return transform_; }
auto ModelInstance::model() const -> uint32_t { return model_; }
auto ModelInstance::visible() const -> bool { return visible_; }
auto ModelInstance::customMaterial() const -> uint32_t { return customMatId; }

auto ModelInstance::setVisible(bool visible) -> void {
  visible_ = visible;
  for(auto &inst: meshInstDescs)
    inst.ptr->visible = visible;
}
auto ModelInstance::setTransform(const Transform &transform) -> void {
  transform_ = transform;
  *transf.ptr = transform;
}
auto ModelInstance::changeModel(uint32_t model) -> void {
  model_ = model;

  auto meshInsDescIdx = 0;
  for(auto &m = scene.model(model_); const auto &nodeId: m.nodes())
    for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
      if(meshId == nullIdx) continue;
      auto &mesh = scene.mesh(meshId);
      auto &primitive = scene.primitive(mesh.primitive());
      auto &material = scene.material(mesh.material());

      auto meshInsDesc = meshInstDescs.at(meshInsDescIdx++);
      *meshInsDesc.ptr = {
        customMatId != nullIdx ? customMatId : material.descOffset(),
        primitive.descOffset(),
        node.transfOffset(),
        transf.offset,
        visible_,
        scene.addToDrawGroup(meshId, meshInsDesc.ptr->drawGroupID)};
    }
}
auto ModelInstance::setCustomMaterial(uint32_t materialId) -> void {
  if(materialId != nullIdx) {
    auto &mat = scene.material(materialId);
    for(auto &inst: meshInstDescs)
      inst.ptr->materialDesc = mat.descOffset();
  } else {
    auto meshInsDescIdx = 0;
    for(auto &m = scene.model(model_); const auto &nodeId: m.nodes())
      for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
        if(meshId == nullIdx) continue;
        auto &mesh = scene.mesh(meshId);
        auto &primitive = scene.primitive(mesh.primitive());
        auto &material = scene.material(mesh.material());

        auto meshInsDesc = meshInstDescs.at(meshInsDescIdx++);
        meshInsDesc.ptr->materialDesc = material.descOffset();
        meshInsDesc.ptr->drawGroupID =
          scene.addToDrawGroup(meshId, meshInsDesc.ptr->drawGroupID);
      }
  }
}

}
