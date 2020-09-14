#version 450
#extension GL_GOOGLE_include_directive : require
#include "raster_resources.h"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;

layout(location = 0) out vec2 outUV0;
layout(location = 1) out flat uint outMaterialID;

layout(push_constant) uniform PushConstant { uint frame; };

void main() {
  MeshInstanceDesc mesh = meshInstances[gl_InstanceIndex];
  mat4 model = matrices[gl_InstanceIndex];
  vec4 pos = model * vec4(inPos, 1.0);
  pos = pos / pos.w;
  outUV0 = inUV0;
  outMaterialID =frameRef(mesh.material, frame);
  gl_Position = camera.projView * pos;
}
