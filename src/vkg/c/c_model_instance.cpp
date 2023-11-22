#include "c_model_instance.h"
#include "vkg/render/scene.hpp"
#include <cstring>

using namespace vkg;
uint32_t ModelInstanceGetCount(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    return modelInstance_.count();
}
void ModelInstanceGetTransform(CScene *scene, uint32_t id, ctransform *transform) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    auto transform_ = modelInstance_.transform();
    memcpy(transform, &transform_, sizeof(transform_));
}
void ModelInstanceSetTransform(CScene *scene, uint32_t id, ctransform *transform) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    modelInstance_.setTransform(*(Transform *)transform);
}
uint32_t ModelInstanceGetModel(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    return modelInstance_.model();
}
bool ModelInstanceGetVisible(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    return modelInstance_.visible();
}
void ModelInstanceSetVisible(CScene *scene, uint32_t id, bool visible) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    modelInstance_.setVisible(visible);
}
void ModelInstanceChangeModel(CScene *scene, uint32_t id, uint32_t model) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    modelInstance_.changeModel(model);
}
uint32_t ModelInstanceGetCustomMaterial(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    return modelInstance_.customMaterial();
}
void ModelInstanceSetCustomMaterial(CScene *scene, uint32_t id, uint32_t materialId) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &modelInstance_ = scene_->modelInstance(id);
    modelInstance_.setCustomMaterial(materialId);
}
