#version 450
#extension GL_GOOGLE_include_directive : require
#include "../../tonemap.h"
#define RASTER_ONLY
#include "../resources.h"

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inUV0;
layout(location = 2) in flat uint inMaterialID;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 pos = cam.projView * vec4(inWorldPos, 1.0);
  float d = texture(depth, gl_FragCoord.xy / vec2(textureSize(depth, 0))).r;
  if(pos.z >= d) discard;
  MaterialUBO material = materials[inMaterialID];
  vec3 albedo = material.colorTex != nullIdx ?
                  SRGBtoLINEAR4(texture(textures[material.colorTex], inUV0)).rgb :
                  vec3(1, 1, 1);
  albedo = material.baseColorFactor.rgb * albedo;
  outColor.rgb = LINEARtoSRGB(albedo);
}