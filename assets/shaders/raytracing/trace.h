#ifndef VKG_RT_TRACE_H
#define VKG_RT_TRACE_H

bool shadowTrace(vec3 origin, vec3 lightDir, float maxDistance) {
  if(--prd.recursion <= 0) return false;
  const uint shadowRayFlags = gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV |
                              gl_RayFlagsSkipClosestHitShaderNV;
  const uint cullMask = 0xFF;
  const float tmin = 0.0;
  const float tmax = maxDistance;
  prdShadow.shadowed = true;
  traceNV(Scene, shadowRayFlags, cullMask, 1, 1, 1, origin, tmin, lightDir, tmax, 2);
  ++prd.recursion;
  return prdShadow.shadowed;
}

void trace(vec3 origin, vec3 dir, float tmin, float tmax) {
  const uint rayFlags = gl_RayFlagsOpaqueNV;
  const uint cullMask = 0xFF;

  traceNV(Scene, rayFlags, cullMask, 0, 1, 0, origin, tmin, dir, tmax, 0);
}

vec3 reflectTrace(vec3 origin, vec3 rayDir, vec3 N) {
  if(--prd.recursion <= 0) return vec3(0);
  vec3 dir = reflect(rayDir, N);
  const float tmin = 1e-3;
  const float tmax = 1e9;
  trace(origin, dir, tmin, tmax);
  ++prd.recursion;
  return prd.color.xyz;
}

vec3 refractTrace(vec3 origin, vec3 rayDir, vec3 N, float eta, vec3 failColor) {
  if(--prd.recursion <= 0) return failColor;
  const float NdotD = dot(N, rayDir);

  vec3 refrNormal = N;
  float refrEta;
  if(NdotD > 0.0f) {
    refrNormal = -N;
    refrEta = 1.0f / eta;
  } else {
    refrNormal = N;
    refrEta = eta;
  }
  vec3 dir = refract(rayDir, refrNormal, refrEta);
  const float tmin = 1e-3;
  const float tmax = 1e9;
  trace(origin, dir, tmin, tmax);
  ++prd.recursion;
  return prd.color.xyz / (refrEta * refrEta);
}

#endif //VKG_RT_TRACE_H
