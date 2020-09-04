#include "c_vec.h"
#include "vkg/render/model/aabb.hpp"
using namespace vkg;

void AABBMergePoint(caabb *aabb, cvec3 *p, uint32_t offset) {
  auto *aabb_ = reinterpret_cast<AABB *>(aabb);
  aabb_->merge(*(glm::vec3 *)((float *)p + offset));
}
void AABBMergeAABB(caabb *aabb, caabb *other) {
  auto *aabb_ = reinterpret_cast<AABB *>(aabb);
  aabb_->merge(*(AABB *)other);
}
void AABBTransformMatrix(caabb *aabb, cmat4 *matrix, uint32_t offset) {
  auto *aabb_ = reinterpret_cast<AABB *>(aabb);
  *aabb_ = aabb_->transform(*(glm::mat4 *)((float *)matrix + offset));
}
void AABBTransform(caabb *aabb, ctransform *transform) {
  auto *aabb_ = reinterpret_cast<AABB *>(aabb);
  *aabb_ = aabb_->transform(*(Transform *)transform);
}