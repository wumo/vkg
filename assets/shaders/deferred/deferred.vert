#version 450
#extension GL_GOOGLE_include_directive : require
#include "resources.h"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV0;
layout(location = 3) out flat uint outMaterialID;

void main() {
  MeshInstanceUBO mesh = meshInstances[gl_InstanceIndex];
  mat4 model = matrices[gl_InstanceIndex];
  vec4 pos = model * vec4(inPos, 1.0);
  pos = pos / pos.w;
  outWorldPos = pos.xyz;
  outNormal = normalize(transpose(inverse(mat3(model))) * inNormal);
  outUV0 = inUV0;
  outMaterialID = mesh.material;
  gl_Position = cam.projView * vec4(outWorldPos, 1.0);
}
