#ifndef VKG_RT_COMMON_H
#define VKG_RT_COMMON_H
#include "../common.h"
#include "../tonemap.h"
#include "../math.h"

struct RayDesc {
  vec3 origin;
  vec3 direction;
  float tmin;
  float tmax;
  uint rayFlags;
  uint cullMask;
  uint sbtRecordOffset;
  uint sbtRecordStride;
  uint missIndex;
};
#define trace(tlas, ray, payload)                                               \
  traceNV(                                                                      \
    tlas, ray.rayFlags, ray.cullMask, ray.sbtRecordOffset, ray.sbtRecordStride, \
    ray.missIndex, ray.origin, ray.tmin, ray.direction, ray.tmax, payload);

struct RayPayload {
  vec3 result;
  vec3 radiance;
  vec3 attenuation;
  vec3 origin;
  vec3 direction;
  uint seed;
  int depth;
  int done;
};

struct ShadowRayPayload {
  bool shadowed;
};

struct ProcedurePayload {
  Vertex v0, v1, v2;
  vec2 uv;
};

const uint RTTriangleFacingCullDisable = 0x01;

struct RTGeometryInstance {
  mat3x4 transform; // row-major 3x4
  uint id_mask;     // [mask:8][id:24]
  uint shader_flag; // [flag:8][shader:24]
  uint64_t handle;
};

#endif //VKG_RT_COMMON_H
