#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#define USE_ATMOSPHERE
#include "resources.h"
#include "../atmosphere/lighting.h"

layout(location = 0) rayPayloadInNV PathTracingRayPayload prd;

void main() {
  vec3 view_direction = gl_WorldRayDirectionNV;
  vec3 backgroundColor = skyBackground(gl_WorldRayOriginNV, view_direction);
  prd.contribution = backgroundColor;
  prd.flags = FLAG_DONE;
  prd.hitT = -1;
}
