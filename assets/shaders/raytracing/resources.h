#ifndef VKG_RAYTRACING_RESOURCES_H
#define VKG_RAYTRACING_RESOURCES_H

#extension GL_EXT_scalar_block_layout : enable

#include "rt_common.h"

layout(constant_id = 0) const uint maxNumTextures = 1;
layout(constant_id = 1) const uint maxLights = 1;
layout(constant_id = 2) const int MAX_RECURSION = 1;
layout(constant_id = 3) const uint NBSamples = 1;

layout(set = 0, binding = 0, scalar) readonly buffer Camera { CameraUBO cam; };
layout(set = 0, binding = 1, scalar) readonly buffer MeshesBuffer {
  MeshInstanceUBO meshInstances[];
};
layout(set = 0, binding = 2, scalar) readonly buffer PrimitiveBuffer {
  PrimitiveUBO primitives[];
};
layout(set = 0, binding = 3, scalar) readonly buffer TransformBuffer {
  Transform transforms[];
};
layout(set = 0, binding = 4, scalar) readonly buffer MaterialBuffer {
  MaterialUBO materials[];
};
layout(set = 0, binding = 5) uniform sampler2D textures[maxNumTextures];

layout(set = 0, binding = 6) uniform LightingUBO { LightUBO lighting; };
layout(set = 0, binding = 7, std430) readonly buffer LightsBuffer {
  LightInstanceUBO lights[maxLights];
};

#ifndef NO_RAYTRACING

layout(set = 1, binding = 0, scalar) readonly buffer PositionBuffer { vec3 positions[]; };
layout(set = 1, binding = 1, scalar) readonly buffer NormalBuffer { vec3 normals[]; };
layout(set = 1, binding = 2, std430) readonly buffer UVBuffer { vec2 uvs[]; };
layout(set = 1, binding = 3, std430) readonly buffer IndexBuffer { uint indices[]; };

layout(set = 1, binding = 4) uniform accelerationStructureNV Scene;
layout(set = 1, binding = 5, rgba8) uniform image2D colorImg;
layout(set = 1, binding = 6, r32f) uniform image2D depthImg;
#endif

#ifdef USE_ATMOSPHERE
  #define ATMOSPHERE_SET 2
  #include "../atmosphere/resources.h"
#endif

layout(set = 3, binding = 0) uniform sampler2D depth;

#endif //VKG_RAYTRACING_RESOURCES_H
