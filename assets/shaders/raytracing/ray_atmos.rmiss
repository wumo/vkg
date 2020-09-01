#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#define USE_ATMOSPHERE
#include "resources.h"
#include "../atmosphere/lighting.h"

layout(location = 0) rayPayloadInNV RayPayload prd;

void main() {
  const vec3 backgroundColor = skyBackground(gl_WorldRayOriginNV, gl_WorldRayDirectionNV);
  prd.radiance = backgroundColor;
  prd.done = 1;
}
