#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rt_common.h"

layout(location = 2) rayPayloadInNV PathTracingShadowRayPayload ShadowRay;

void main() {
    ShadowRay.isHit = false;
}
