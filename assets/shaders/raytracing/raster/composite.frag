#version 450
#extension GL_GOOGLE_include_directive : require
#include "raster_resources.h"

layout(location = 0) in vec2 inUV0;
layout(location = 1) in flat uint inMaterialID;
layout(location = 2) in float inDepth;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outReveal;

void main() {
  MaterialDesc material = materials[inMaterialID];
  vec4 albedo = material.colorTex != nullIdx ?
                  SRGBtoLINEAR4(texture(textures[material.colorTex], inUV0)) :
                  vec4(1, 1, 1, 1);
  albedo = material.baseColorFactor.rgba * albedo;
  if(albedo.a < material.alphaCutoff) discard;
  vec4 color = albedo;

  color.rgb *= color.a; // Premultiply it

  // Insert your favorite weighting function here. The color-based factor
  // avoids color pollution from the edges of wispy clouds. The z-based
  // factor gives precedence to nearer surfaces.

  // The depth functions in the paper want a camera-space depth of 0.1 < z < 500,
  // but the scene at the moment uses a range of about 0.01 to 50, so multiply
  // by 10 to get an adjusted depth:
  const float depthZ = -inDepth * 10.0f;

  const float distWeight = clamp(0.03 / (1e-5 + pow(depthZ / 200, 4.0)), 1e-2, 3e3);

  float alphaWeight =
    min(1.0, max(max(color.r, color.g), max(color.b, color.a)) * 40.0 + 0.01);
  alphaWeight *= alphaWeight;

  const float weight = alphaWeight * distWeight;

  // Blend function: GL_ONE, GL_ONE
  outColor = color * weight;
  // GL blend function: GL_ZERO, GL_ONE_MINUS_SRC_ALPHA
  outReveal.r = color.a;
}