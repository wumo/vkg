#version 450
#extension GL_GOOGLE_include_directive : require
#include "raster_resources.h"

layout(location = 0) in vec2 inUV0;
layout(location = 1) in flat uint inMaterialID;

layout(location = 0) out vec4 outColor;

void main() {
  MaterialDesc material = materials[inMaterialID];
  vec4 albedo = material.colorTex != nullIdx ?
                  SRGBtoLINEAR4(texture(textures[material.colorTex], inUV0)) :
                  vec4(1, 1, 1, 1);
  albedo = material.baseColorFactor.rgba * albedo;
  outColor = albedo;
}