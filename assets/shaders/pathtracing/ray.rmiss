#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "resources.h"

layout(location = 0) rayPayloadInNV PathTracingRayPayload prd;

void main() {
  const vec3 backgroundColor = vec3(0.412f, 0.796f, 1.0f);
  prd.contribution = vec3(0, 0, 0);
  prd.flags = FLAG_DONE;
  prd.hitT = -1;
}
