#include "c_scene.h"
#include "vkg/render/scene.hpp"
#include "vkg/base/resource/texture_formats.hpp"
#include <cstring>

using namespace vkg;
uint32_t SceneNewPrimitive(
  CScene *scene, cvec3 *positions, uint32_t position_offset_float, uint32_t numPositions,
  cvec3 *normals, uint32_t normal_offset_float, uint32_t numNormals, cvec2 *uvs,
  uint32_t uv_offset_float, uint32_t numUVs, uint32_t *indices, uint32_t numIndices,
  caabb *aabb, CPrimitiveTopology topology, bool perFrame) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto *aabb_ = (AABB *)aabb;
  return scene_->newPrimitive(
    {(Vertex::Position *)((float *)positions + position_offset_float), numPositions},
    {(Vertex::Normal *)((float *)normals + normal_offset_float), numNormals},
    {(Vertex::UV *)((float *)uvs + uv_offset_float), numUVs}, {indices, numIndices},
    *aabb_, static_cast<PrimitiveTopology>(topology), perFrame);
}
void SceneNewPrimitives(
  CScene *scene, CPrimitiveBuilder *builder, bool perFrame, uint32_t *ptrs) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto primitives =
    scene_->newPrimitives(*reinterpret_cast<PrimitiveBuilder *>(builder), perFrame);
  memcpy(ptrs, primitives.data(), primitives.size() * sizeof(uint32_t));
}
uint32_t SceneNewMaterial(CScene *scene, CMaterialType type, bool perFrame) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newMaterial(static_cast<MaterialType>(type), perFrame);
}
uint32_t SceneNewTexture(CScene *scene, char *pathBuf, uint32_t pathSize, bool mipmap) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newTexture(std::string{pathBuf, pathSize}, mipmap);
}
uint32_t SceneNewTextureFromBytes(
  CScene *scene, const char *bytes, uint32_t numBytes, uint32_t width, uint32_t height,
  TextureFormat format, bool mipmap) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newTexture(
    {(std::byte *)bytes, numBytes}, width, height, image::toVulkanTextureFormat(format),
    mipmap);
}
uint32_t SceneNewMesh(CScene *scene, uint32_t primitive, uint32_t material) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newMesh(primitive, material);
}
uint32_t SceneNewNode(CScene *scene, ctransform *transform) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newNode(*(Transform *)transform);
}
uint32_t SceneNewModel(CScene *scene, uint32_t *nodes, uint32_t numNodes) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  std::vector<uint32_t> nodes_;
  nodes_.reserve(numNodes);
  for(int i = 0; i < numNodes; ++i)
    nodes_.push_back(nodes[i]);
  return scene_->newModel(std::move(nodes_));
}
uint32_t SceneNewLight(CScene *scene, bool perFrame) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newLight(perFrame);
}
uint32_t SceneLoadModel(
  CScene *scene, char *pathBuf, uint32_t pathSize, CMaterialType type) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->loadModel(
    std::string(pathBuf, pathSize), static_cast<MaterialType>(type));
}
uint32_t SceneLoadModelFromBytes(
  CScene *scene, const char *bytes, uint32_t numBytes, uint32_t pathSize,
  CMaterialType type) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->loadModel(
    {(std::byte *)bytes, numBytes}, static_cast<MaterialType>(type));
}
uint32_t SceneNewModelInstance(
  CScene *scene, uint32_t model, ctransform *transform, bool perFrame) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return scene_->newModelInstance(model, *(Transform *)transform, perFrame);
}
CCamera *SceneGetCamera(CScene *scene) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return reinterpret_cast<CCamera *>(&scene_->camera());
}
CAtmosphereSetting *SceneGetAtmosphere(CScene *scene) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return reinterpret_cast<CAtmosphereSetting *>(&scene_->atmosphere());
}
CShadowMapSetting *SceneGetShadowmap(CScene *scene) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  return reinterpret_cast<CShadowMapSetting *>(&scene_->shadowmap());
}
