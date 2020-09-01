#ifndef VKG_RT_TRACE_H
#define VKG_RT_TRACE_H

#include "rt_common.h"

//bool shadowTrace(vec3 origin, vec3 lightDir, float maxDistance) {
//  RayDesc ray;
//  ray.origin = origin;
//  ray.direction = lightDir;
//  ray.tmin = 0.0;
//  ray.tmax = maxDistance;
//  ray.rayFlags = gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV |
//                 gl_RayFlagsSkipClosestHitShaderNV;
//  ray.cullMask = opaqueMask;
//  ray.sbtRecordOffset = 1;
//  ray.sbtRecordStride = 0;
//  ray.missIndex = 1;
//  prdShadow.shadowed = true;
//  trace(tlas, ray, 1);
//  return prdShadow.shadowed;
//}
//
//vec3 reflectTrace(vec3 origin, vec3 rayDir, vec3 N) {
//  if(--prd.recursion <= 0) return vec3(0);
//  vec3 dir = reflect(rayDir, N);
//  const float tmin = 1e-3;
//  const float tmax = 1e9;
//  trace(origin, dir, tmin, tmax);
//  ++prd.recursion;
//  return prd.color.xyz;
//}
//
//vec3 refractTrace(vec3 origin, vec3 rayDir, vec3 N, float eta, vec3 failColor) {
//  if(--prd.recursion <= 0) return failColor;
//  const float NdotD = dot(N, rayDir);
//
//  vec3 refrNormal = N;
//  float refrEta;
//  if(NdotD > 0.0f) {
//    refrNormal = -N;
//    refrEta = 1.0f / eta;
//  } else {
//    refrNormal = N;
//    refrEta = eta;
//  }
//  vec3 dir = refract(rayDir, refrNormal, refrEta);
//  const float tmin = 1e-3;
//  const float tmax = 1e9;
//  trace(origin, dir, tmin, tmax);
//  ++prd.recursion;
//  return prd.color.xyz / (refrEta * refrEta);
//}

#endif //VKG_RT_TRACE_H
