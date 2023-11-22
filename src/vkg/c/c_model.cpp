#include "c_model.h"

#include "vkg/render/scene.hpp"
#include <cstring>

using namespace vkg;

uint32_t ModelNumNodes(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &model = scene_->model(id);
    return model.nodes().size();
}
void ModelGetNodes(CScene *scene, uint32_t id, uint32_t *nodes) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &model = scene_->model(id);
    auto nodes_ = model.nodes();
    memcpy(nodes, nodes_.data(), nodes_.size_bytes());
}
void ModelGetAABB(CScene *scene, uint32_t id, caabb *aabb) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &model = scene_->model(id);
    auto aabb_ = model.aabb();
    memcpy(aabb, &aabb_, sizeof(aabb_));
}
uint32_t ModelNumAnimations(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &model = scene_->model(id);
    return model.animations().size();
}
CAnimation *ModelGetAnimation(CScene *scene, uint32_t id, uint32_t idx) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &model = scene_->model(id);
    return reinterpret_cast<CAnimation *>(model.animations().data() + idx);
}

void AnimationReset(CAnimation *animation, uint32_t idx) {
    auto *anim = reinterpret_cast<Animation *>(animation);
    anim->reset(idx);
}
void AnimationResetAll(CAnimation *animation) {
    auto *anim = reinterpret_cast<Animation *>(animation);
    anim->resetAll();
}
void AnimationAnimate(CAnimation *animation, uint32_t idx, float elapsedMs) {
    auto *anim = reinterpret_cast<Animation *>(animation);
    anim->animate(idx, elapsedMs);
}
void AnimationAnimateAll(CAnimation *animation, float elapsedMs) {
    auto *anim = reinterpret_cast<Animation *>(animation);
    anim->animateAll(elapsedMs);
}