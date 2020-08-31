#ifndef VKG_RAYTRACING_RESOURCES_H
#define VKG_RAYTRACING_RESOURCES_H

#extension GL_EXT_scalar_block_layout : enable

#include "rt_common.h"

layout(constant_id = 0) const uint maxNumTextures = 1;

layout(push_constant) uniform PushConstant {
  uint maxDepth;
  uint nbSamples;
  uint frame;
};

layout(set = 0, binding = 0) uniform accelerationStructureNV tlas;
layout(set = 0, binding = 1, rgba8) uniform image2D colorImg;
layout(set = 0, binding = 2, r32f) uniform image2D depthImg;

layout(set = 0, binding = 3, scalar) readonly buffer Camera { CameraUBO cam; };
layout(set = 0, binding = 4, scalar) readonly buffer MeshesBuffer {
  MeshInstanceUBO meshInstances[];
};

layout(set = 0, binding = 5, scalar) readonly buffer PrimitiveBuffer {
  PrimitiveUBO primitives[];
};
layout(set = 0, binding = 6, scalar) readonly buffer PositionBuffer { vec3 positions[]; };
layout(set = 0, binding = 7, scalar) readonly buffer NormalBuffer { vec3 normals[]; };
layout(set = 0, binding = 8, std430) readonly buffer UVBuffer { vec2 uvs[]; };
layout(set = 0, binding = 9, std430) readonly buffer IndexBuffer { uint indices[]; };

layout(set = 0, binding = 10, scalar) readonly buffer MaterialBuffer {
  MaterialUBO materials[];
};
layout(set = 0, binding = 11) uniform sampler2D textures[maxNumTextures];

layout(set = 0, binding = 12) uniform LightingUBO { LightUBO lighting; };
layout(set = 0, binding = 13, std430) readonly buffer LightsBuffer {
  LightInstanceUBO lights[];
};

#ifdef USE_ATMOSPHERE
  #define ATMOSPHERE_SET 2
  #include "../atmosphere/resources.h"
#endif

  //layout(set = 3, binding = 0) uniform sampler2D depth;

#endif //VKG_RAYTRACING_RESOURCES_H
