#ifndef VKG_C_NODE_H
#define VKG_C_NODE_H
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CScene;

uint32_t NodeNameLength(CScene *scene, uint32_t id);
void NodeGetName(CScene *scene, uint32_t id, char *buf);
void NodeSetName(CScene *scene, uint32_t id, char *buf, uint32_t size);
void NodeGetTransform(CScene *scene, uint32_t id, ctransform *transform);
void NodeSetTransform(CScene *scene, uint32_t id, ctransform *transform);
uint32_t NodeNumMeshes(CScene *scene, uint32_t id);
void NodeGetMeshes(CScene *scene, uint32_t id, uint32_t *meshes);
void NodeAddMeshes(CScene *scene, uint32_t id, uint32_t *meshes, uint32_t numMeshes);
uint32_t NodeGetParent(CScene *scene, uint32_t id);
uint32_t NodeNumChildren(CScene *scene, uint32_t id);
void NodeGetChildren(CScene *scene, uint32_t id, uint32_t *children);
void NodeAddChildren(
  CScene *scene, uint32_t id, uint32_t *children, uint32_t numChildren);
void NodeGetAABB(CScene *scene, uint32_t id, caabb *aabb);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_NODE_H
