#ifndef VKG_RASTER_RESOURCES_H
#define VKG_RASTER_RESOURCES_H

#extension GL_EXT_scalar_block_layout : enable

#include "../../common.h"
#include "../../tonemap.h"

layout(constant_id = 0) const uint maxNumTextures = 1;

layout(set = 0, binding = 0) uniform sampler2D depthImg;

layout(set = 0, binding = 1, scalar) readonly buffer Camera { CameraDesc camera; };
layout(set = 0, binding = 2, scalar) readonly buffer MeshesBuf {
  MeshInstanceDesc meshInstances[];
};
layout(set = 0, binding = 3, scalar) readonly buffer PrimitiveBuf {
  PrimitiveDesc primitives[];
};
layout(set = 0, binding = 4, std430) readonly buffer TransformBuf { mat4 matrices[]; };
layout(set = 0, binding = 5, scalar) readonly buffer MaterialBuf {
  MaterialDesc materials[];
};
layout(set = 0, binding = 6) uniform sampler2D textures[maxNumTextures];

#endif //VKG_RASTER_RESOURCES_H
