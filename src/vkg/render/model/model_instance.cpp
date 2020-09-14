#include "model_instance.hpp"
#include "vkg/render/scene.hpp"
namespace vkg {

ModelInstance::ModelInstance(
  Scene &scene, uint32_t id, const Transform &transform, uint32_t modelId, uint32_t count)
  : scene{scene}, id_{id}, count_{count}, model_{modelId}, transform_{transform} {
  transfs.resize(count);
  for(auto i = 0u; i < count; i++) {
    transfs[i] = scene.allocateTransform();
    *transfs[i].ptr = transform;
  }

  for(auto &m = scene.model(modelId); const auto &nodeId: m.nodes())
    for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
      if(meshId == nullIdx) continue;
      auto &mesh = scene.mesh(meshId);
      auto &primitive = scene.primitive(mesh.primitive());
      auto &material = scene.material(mesh.material());

      auto meshInsDesc = scene.allocateMeshInstDesc();
      auto drawGroup = scene.addToDrawGroup(meshId);
      *meshInsDesc.ptr = {
        {material.descOffset(), material.count()},
        {primitive.descOffset(), primitive.count()},
        node.transfOffset(),
        {transfs[0].offset, uint32_t(transfs.size())},
        true,
        drawGroup};
      meshInstDescs.push_back({drawGroup, meshInsDesc});
    }
}
auto ModelInstance::id() const -> uint32_t { return id_; }
auto ModelInstance::count() const -> uint32_t { return count_; }
auto ModelInstance::transform() const -> Transform { return transform_; }
auto ModelInstance::model() const -> uint32_t { return model_; }
auto ModelInstance::visible() const -> bool { return visible_; }

auto ModelInstance::customMaterial() const -> uint32_t { return customMatId; }
auto ModelInstance::setVisible(bool visible) -> void {
  visible_ = visible;
  for(auto &inst: meshInstDescs) {
    inst.desc.ptr->visible = visible;
    scene.setVisible(inst.shadeModel, visible);
  }
}
auto ModelInstance::setTransform(const Transform &transform) -> void {
  transform_ = transform;
  scene.scheduleFrameUpdate(Update::Type::Instance, id_, count_, ticket);
}
auto ModelInstance::changeModel(uint32_t model) -> void {
  model_ = model;

  auto meshInsDescIdx = 0;
  for(auto &m = scene.model(model_); const auto &nodeId: m.nodes())
    for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
      if(meshId == nullIdx) continue;
      auto &mesh = scene.mesh(meshId);
      auto &primitive = scene.primitive(mesh.primitive());
      auto &material =
        scene.material(customMatId != nullIdx ? customMatId : mesh.material());

      auto meshInsDesc = meshInstDescs.at(meshInsDescIdx++);
      meshInsDesc.shadeModel = scene.addToDrawGroup(meshId, meshInsDesc.shadeModel);
      *meshInsDesc.desc.ptr = {material.descOffset(),    material.count(),
                               primitive.descOffset(),   primitive.count(),
                               node.transfOffset(),      transfs[0].offset,
                               uint32_t(transfs.size()), visible_,
                               meshInsDesc.shadeModel};
    }
}
auto ModelInstance::setCustomMaterial(uint32_t materialId) -> void {
  if(materialId != nullIdx) {
    auto &mat = scene.material(materialId);
    for(auto &inst: meshInstDescs)
      inst.desc.ptr->materialDesc.idx = mat.descOffset();
  } else {
    auto meshInsDescIdx = 0;
    for(auto &m = scene.model(model_); const auto &nodeId: m.nodes())
      for(auto &node = scene.node(nodeId); const auto &meshId: node.meshes()) {
        if(meshId == nullIdx) continue;
        auto &mesh = scene.mesh(meshId);
        auto &primitive = scene.primitive(mesh.primitive());
        auto &material = scene.material(mesh.material());

        //TODO check
        auto meshInsDesc = meshInstDescs.at(meshInsDescIdx++);
        meshInsDesc.shadeModel = scene.addToDrawGroup(meshId, meshInsDesc.shadeModel);
        meshInsDesc.desc.ptr->materialDesc.idx = material.descOffset();
        meshInsDesc.desc.ptr->materialDesc.count = material.count();
        meshInsDesc.desc.ptr->shadeModel = meshInsDesc.shadeModel;
      }
  }
  customMatId = materialId;
}
void ModelInstance::updateFrame(uint32_t frameIdx, vk::CommandBuffer commandBuffer) {
  *transfs[std::clamp(frameIdx, 0u, count_ - 1)].ptr = transform_;
}

}
