#include "c_mesh.h"

#include "vkg/render/scene.hpp"

using namespace vkg;

uint32_t MeshGetPrimitive(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &mesh = scene_->mesh(id);
    return mesh.primitive();
}
uint32_t MeshGetMaterial(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &mesh = scene_->mesh(id);
    return mesh.material();
}