#ifndef VKG_DEFERRED_COMMON_H
#define VKG_DEFERRED_COMMON_H

#include "../common.h"

struct CascadeDesc {
  mat4 lightViewProj;
  vec3 lightDir;
  float z;
};

struct ShadowMapSetting {
  uint numCascades;
};

#endif //VKG_DEFERRED_COMMON_H
