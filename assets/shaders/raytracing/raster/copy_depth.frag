#version 450
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D depthImg;

void main() {
  float d = texture(depthImg, inUV).r;
  gl_FragDepth = d;
}