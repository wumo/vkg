#ifndef VKG_C_MESH_H
#define VKG_C_MESH_H

#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CScene;

uint32_t MeshGetPrimitive(CScene *scene, uint32_t id);
uint32_t MeshGetMaterial(CScene *scene, uint32_t id);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_MESH_H
