#version 450
#extension GL_GOOGLE_include_directive : require
#define SUBPASS_COMPOSITE
#include "resources.h"

layout(location = 0) in vec2 inUV0;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 accum = subpassLoad(texColor);
  float reveal = subpassLoad(texWeights).r;

  outColor = vec4(accum.rgb / max(accum.a, 1e-5), reveal);
}