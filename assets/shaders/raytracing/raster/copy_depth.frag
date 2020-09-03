#version 450
#extension GL_GOOGLE_include_directive : require

#include "raster_resources.h"

layout(location = 0) in vec2 inUV;

void main() {
  float d = texture(depthImg, inUV).r;
  gl_FragDepth = d;
}