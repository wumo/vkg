#ifndef VKG_C_LIGHT_H
#define VKG_C_LIGHT_H
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CScene;

uint32_t LightGetCount(CScene *scene, uint32_t id);
void LightGetColor(CScene *scene, uint32_t id, cvec3 *color, uint32_t offset_float);
void LightSetColor(CScene *scene, uint32_t id, cvec3 *color, uint32_t offset_float);
void LightGetLocation(CScene *scene, uint32_t id, cvec3 *location, uint32_t offset_float);
void LightSetLocation(CScene *scene, uint32_t id, cvec3 *location, uint32_t offset_float);
float LightGetIntensity(CScene *scene, uint32_t id);
void LightSetIntensity(CScene *scene, uint32_t id, float intensity);
float LightGetRange(CScene *scene, uint32_t id);
void LightSetRange(CScene *scene, uint32_t id, float range);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_LIGHT_H
