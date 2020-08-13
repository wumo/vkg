#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : enable

#include "../deferred_common.h"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;

layout(constant_id = 0) const uint numCascades = 1;
layout(push_constant) uniform CascadedIndex { uint cascadeIndex; };
layout(set = 0, binding = 0) buffer Cascades { CascadeDesc cascades[]; };
layout(set = 0, binding = 1, std430) buffer TransformMatrixBuffer { mat4 matrices[]; };

void main() {
  mat4 model = matrices[gl_InstanceIndex];
  vec4 pos = model * vec4(inPos, 1.0);
  pos = pos / pos.w;
  gl_Position = cascades[cascadeIndex].lightViewProj * pos;
}
