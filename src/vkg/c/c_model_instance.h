#ifndef VKG_C_MODEL_INSTANCE_H
#define VKG_C_MODEL_INSTANCE_H

#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#else
  #include <stdbool.h>
#endif

struct CScene;

uint32_t ModelInstanceGetCount(CScene *scene, uint32_t id);
void ModelInstanceGetTransform(CScene *scene, uint32_t id, ctransform *transform);//TODO offset_float?
void ModelInstanceSetTransform(CScene *scene, uint32_t id, ctransform *transform);
uint32_t ModelInstanceGetModel(CScene *scene, uint32_t id);
bool ModelInstanceGetVisible(CScene *scene, uint32_t id);
void ModelInstanceSetVisible(CScene *scene, uint32_t id, bool visible);
void ModelInstanceChangeModel(CScene *scene, uint32_t id, uint32_t model);
uint32_t ModelInstanceGetCustomMaterial(CScene *scene, uint32_t id);
void ModelInstanceSetCustomMaterial(CScene *scene, uint32_t id, uint32_t materialId);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_MODEL_INSTANCE_H
