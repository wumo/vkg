#version 450
#extension GL_GOOGLE_include_directive : require
#include "raster_resources.h"

layout(location = 0) in vec2 inUV0;

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput texColor;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput texWeights;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 accum = subpassLoad(texColor);
  float reveal = subpassLoad(texWeights).r;

  outColor = vec4(accum.rgb / max(accum.a, 1e-5), reveal);
}