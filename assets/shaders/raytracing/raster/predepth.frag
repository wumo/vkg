#version 450
#extension GL_GOOGLE_include_directive : require
#define NO_RAYTRACING
#include "../resources.h"

void main() {
  float d = texture(depth, gl_FragCoord.xy / vec2(textureSize(depth, 0))).r;
  gl_FragDepth = d;
}