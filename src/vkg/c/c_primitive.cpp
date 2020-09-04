#include "c_primitive.h"
#include "vkg/render/scene.hpp"

using namespace vkg;
uint32_t PrimitiveGetCount(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  return primitive.count();
}
CPrimitiveTopology PrimitiveGetTopology(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  return static_cast<CPrimitiveTopology>(primitive.topology());
}
CUintRange PrimitiveGetIndex(CScene *scene, uint32_t id, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto range = primitive.index(idx);
  return *(CUintRange *)(&range);
}
CUintRange PrimitiveGetPosition(CScene *scene, uint32_t id, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto range = primitive.position(idx);
  return *(CUintRange *)(&range);
}
CUintRange PrimitiveGetNormal(CScene *scene, uint32_t id, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto range = primitive.normal(idx);
  return *(CUintRange *)(&range);
}
CUintRange PrimitiveGetUV(CScene *scene, uint32_t id, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto range = primitive.uv(idx);
  return *(CUintRange *)(&range);
}
caabb PrimitiveGetAABB(CScene *scene, uint32_t id, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto aabb = primitive.aabb(idx);
  return *(caabb *)(&aabb);
}
void PrimitiveSetAABB(CScene *scene, uint32_t id, caabb *aabb, uint32_t idx) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  primitive.setAABB(idx, *(AABB *)aabb);
}
void PrimitiveUpdate(
  CScene *scene, uint32_t id, uint32_t idx, cvec3 *positions,
  uint32_t position_offset_float, uint32_t numPositions, cvec3 *normals,
  uint32_t normal_offset_float, uint32_t numNormals, caabb *aabb) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  auto *aabb_ = (AABB *)aabb;
  primitive.update(
    idx, {(Vertex::Position *)((float *)positions + position_offset_float), numPositions},
    {(Vertex::Normal *)((float *)normals + normal_offset_float), numNormals}, *aabb_);
}
void PrimitiveUpdateFromBuilder(
  CScene *scene, uint32_t id, uint32_t idx, CPrimitiveBuilder *builder) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &primitive = scene_->primitive(id);
  primitive.update(idx, *reinterpret_cast<PrimitiveBuilder *>(builder));
}
