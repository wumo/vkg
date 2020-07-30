#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../tonemap.h"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D denoisedImg;

void main() {
  vec3 color = texture(denoisedImg, inUV).rgb;

  fragColor = vec4(LINEARtoSRGB(color), 1.0f);
}
