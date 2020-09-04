#ifndef VKG_C_SHADOWMAP_HPP
#define VKG_C_SHADOWMAP_HPP

#include "c_vec.h"

#ifdef __cplusplus
extern "C" {
#else
  #include <stdbool.h>
#endif

struct CShadowMapSetting;
typedef struct CShadowMapSetting CShadowMapSetting;

bool ShadowMapIsEnabled(CShadowMapSetting *shadowmap);
void ShadowMapEnable(CShadowMapSetting *shadowmap, bool enabled);

uint32_t ShadowMapGetNumCascades(CShadowMapSetting *shadowmap);
void ShadowMapSetNumCascades(CShadowMapSetting *shadowmap, uint32_t numCascades);

uint32_t ShadowMapGetTextureSize(CShadowMapSetting *shadowmap);
void ShadowMapSetTextureSize(CShadowMapSetting *shadowmap, uint32_t textureSize);

float ShadowMapGetZFar(CShadowMapSetting *shadowmap);
void ShadowMapSetZFar(CShadowMapSetting *shadowmap, float zFar);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_SHADOWMAP_HPP
