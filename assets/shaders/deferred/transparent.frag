#version 450
// #extension GL_EXT_debug_printf : enable
#extension GL_GOOGLE_include_directive : require
#include "../tonemap.h"
#include "resources.h"

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in flat uint inMaterialID;

layout(location = 0) out vec4 outColor;

void main() {
  MaterialUBO material = materials[inMaterialID];
  vec4 albedo = material.colorTex != nullIdx ?
                  SRGBtoLINEAR4(texture(textures[material.colorTex], inUV0)) :
                  vec4(1, 1, 1, 1);
  albedo = material.baseColorFactor.rgba * albedo;
  if(albedo.a < material.alphaCutoff) discard;
  outColor = albedo;
}