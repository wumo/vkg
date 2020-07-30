#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rt_common.h"

layout(location = 0) rayPayloadInNV RayTracingRayPayload prd;

void main() {
  const vec3 backgroundColor = vec3(0.412f, 0.796f, 1.0f);
  prd.color = backgroundColor;
  prd.hitT = -1;
}
