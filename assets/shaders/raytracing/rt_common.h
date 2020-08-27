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

const uint RTTriangleFacingCullDisable = 0x01;

struct RTGeometryInstance {
  mat3x4 transform;
  uint id_mask;     // mask:8 <-> id:24;
  uint shader_flag; // flag:8 <-> shader:24;
  uint64_t handle;
};

#endif //VKG_RT_COMMON_H
