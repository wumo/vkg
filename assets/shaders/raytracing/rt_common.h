#ifndef VKG_RT_COMMON_H
#define VKG_RT_COMMON_H
#include "../common.h"
#include "../tonemap.h"
#include "../math.h"

struct RayTracingRayPayload {
  vec3 color;
  float hitT;
  int recursion;
  uint seed;
};

struct RayTracingShadowRayPayload {
  bool shadowed;
};

struct ProcedurePayload {
  Vertex v0, v1, v2;
  vec2 uv;
};

struct RTGeometryInstance {
  vec4 r1, r2, r3;
  uvec4 other;
};

#endif //VKG_RT_COMMON_H
