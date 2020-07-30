#ifndef VKG_PATHTRACING_RESOURCES_H
#define VKG_PATHTRACING_RESOURCES_H

#extension GL_EXT_scalar_block_layout : enable

#include "rt_common.h"

layout(constant_id = 1) const uint maxNumTextures = 1;
layout(constant_id = 2) const uint maxLights = 1;
layout(constant_id = 0) const int MAX_RECURSION = 3;
layout(constant_id = 3) const uint NBSamples = 1;
layout(constant_id = 4) const float fireflyClampThreshold = 10000.f;
layout(constant_id = 5) const uint debugMode = 0;

layout(set = 0, binding = 0) uniform Camera { CameraUBO cam; };
layout(set = 0, binding = 1, std430) readonly buffer MeshesBuffer {
  MeshInstanceUBO meshInstances[];
};
layout(set = 0, binding = 2, std430) readonly buffer TransformBuffer {
  mat4 transforms[];
};
layout(set = 0, binding = 3, std430) readonly buffer MaterialBuffer {
  MaterialUBO materials[];
};
layout(set = 0, binding = 4) uniform sampler2D textures[maxNumTextures];

layout(set = 0, binding = 5) uniform LightingUBO { LightUBO lighting; };
layout(set = 0, binding = 6, std430) readonly buffer LightsBuffer {
  LightInstanceUBO lights[];
};

#ifndef RASTER_ONLY
layout(set = 1, binding = 0, std430) readonly buffer PrimitiveBuffer {
  PrimitiveUBO primitives[];
};
layout(set = 1, binding = 1, scalar) readonly buffer PositionBuffer { vec3 positions[]; };
layout(set = 1, binding = 2, scalar) readonly buffer NormalBuffer { vec3 normals[]; };
layout(set = 1, binding = 3, std430) readonly buffer UVBuffer { vec2 uvs[]; };
layout(set = 1, binding = 4, std430) readonly buffer IndexBuffer { uint indices[]; };

layout(set = 1, binding = 5) uniform accelerationStructureNV Scene;
layout(set = 1, binding = 6, rgba8) uniform image2D colorImg;
layout(set = 1, binding = 7, rgba8) uniform image2D albedoImg;
layout(set = 1, binding = 8, rgba8) uniform image2D normalImg;
layout(set = 1, binding = 9, r32f) uniform image2D depthImg;
#endif

#ifdef USE_ATMOSPHERE
  #define ATMOSPHERE_SET 2
  #include "../atmosphere/resources.h"
#endif

layout(set = 3, binding = 0) uniform sampler2D depth;

#endif //VKG_RT_RESOURCES_H
