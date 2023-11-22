#include "c_node.h"
#include "vkg/render/scene.hpp"
#include <cstring>
using namespace vkg;
uint32_t NodeNameLength(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    return uint32_t(node.name().size());
}
void NodeGetName(CScene *scene, uint32_t id, char *buf) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    auto name = node.name();
    memcpy(buf, name.c_str(), name.size());
}
void NodeSetName(CScene *scene, uint32_t id, char *buf, uint32_t size) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    node.setName(std::string{buf, size});
}
void NodeGetTransform(CScene *scene, uint32_t id, ctransform *transform) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    auto transform_ = node.transform();
    memcpy(transform, &transform_, sizeof(transform_));
}
void NodeSetTransform(CScene *scene, uint32_t id, ctransform *transform) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    node.setTransform(*(Transform *)transform);
}
uint32_t NodeNumMeshes(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    return node.meshes().size();
}
void NodeGetMeshes(CScene *scene, uint32_t id, uint32_t *meshes) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    auto meshes_ = node.meshes();
    memcpy(meshes, meshes_.data(), meshes_.size_bytes());
}
void NodeAddMeshes(CScene *scene, uint32_t id, uint32_t *meshes, uint32_t numMeshes) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    std::vector<uint32_t> meshes_;
    meshes_.reserve(numMeshes);
    for(int i = 0; i < numMeshes; ++i)
        meshes_.push_back(meshes[i]);
    node.addMeshes(std::move(meshes_));
}
uint32_t NodeGetParent(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    return node.parent();
}
uint32_t NodeNumChildren(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    return node.children().size();
}
void NodeGetChildren(CScene *scene, uint32_t id, uint32_t *children) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    auto children_ = node.children();
    memcpy(children, children_.data(), children_.size_bytes());
}
void NodeAddChildren(CScene *scene, uint32_t id, uint32_t *children, uint32_t numChildren) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    std::vector<uint32_t> children_;
    children_.reserve(numChildren);
    for(int i = 0; i < numChildren; ++i)
        children_.push_back(children[i]);
    node.addChildren(std::move(children_));
}
void NodeGetAABB(CScene *scene, uint32_t id, caabb *aabb) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &node = scene_->node(id);
    auto aabb_ = node.aabb();
    memcpy(aabb, &aabb_, sizeof(aabb_));
}