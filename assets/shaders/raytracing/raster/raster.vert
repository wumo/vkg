#version 450
#extension GL_GOOGLE_include_directive : require
#define NO_RAYTRACING
#include "../resources.h"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec2 outUV0;
layout(location = 2) out flat uint outMaterialID;

void main() {
  MeshInstanceUBO mesh = meshInstances[gl_InstanceIndex];
  mat4 model = toMatrix(transforms[mesh.instance]) * toMatrix(transforms[mesh.node]);
  vec4 pos = model * vec4(inPos, 1.0);
  outWorldPos = pos.xyz / pos.w;
  outUV0 = inUV0;
  outMaterialID = mesh.material;
  gl_Position = cam.projView * vec4(outWorldPos, 1.0);
}
