#ifndef VKG_CSM_H
#define VKG_CSM_H

#include "../resources.h"

uint cascadeIndex(vec3 pos) {
  float z = dot(pos - cam.eye.xyz, cam.v.xyz);

  uint cascadeIdx = 0;
  for(int i = 0; i < shadowMapSeting.numCascades; ++i)
    if(z < cascades[i].z) {
      cascadeIdx = i;
      break;
    }

  return cascadeIdx;
}

float shadowCoordDepth(vec3 pos) {
  if(!shadowMapSeting.enabled) return 1;

  uint cascadeIdx = cascadeIndex(pos);

  CascadeDesc cascade = cascades[cascadeIdx];

  vec4 shadowCoord = cascade.lightViewProj * vec4(pos, 1);
  shadowCoord = shadowCoord / shadowCoord.w;
  //shadowCoord in [-1,1]x[-1,1],map to [0,1]x[0,1] first.
  shadowCoord.xy = shadowCoord.xy / 2 + 0.5;
  
  return shadowCoord.z;
}

float shadowMapDepth(vec3 pos) {
  if(!shadowMapSeting.enabled) return 1;
  
  uint cascadeIdx = cascadeIndex(pos);
  
  CascadeDesc cascade = cascades[cascadeIdx];
  
  vec4 shadowCoord = cascade.lightViewProj * vec4(pos, 1);
  shadowCoord = shadowCoord / shadowCoord.w;
  //shadowCoord in [-1,1]x[-1,1],map to [0,1]x[0,1] first.
  shadowCoord.xy = shadowCoord.xy / 2 + 0.5;
  
  float depth = texture(shadowMap, vec3(shadowCoord.xy, cascadeIdx)).r;
  return depth;
}

bool shadowTrace(vec3 pos) {
  if(!shadowMapSeting.enabled) return false;

  uint cascadeIdx = cascadeIndex(pos);

  CascadeDesc cascade = cascades[cascadeIdx];

  vec4 shadowCoord = cascade.lightViewProj * vec4(pos, 1);
  shadowCoord = shadowCoord / shadowCoord.w;
  //shadowCoord in [-1,1]x[-1,1],map to [0,1]x[0,1] first.
  shadowCoord.xy = shadowCoord.xy / 2 + 0.5;
  
  float depth = texture(shadowMap, vec3(shadowCoord.xy, cascadeIdx)).r;

  float bias = 0.0005;
  return shadowCoord.z > depth + bias;
}

#endif //VKG_CSM_H
