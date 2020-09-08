#ifndef VKG_RAYTRACING_HIT_RCHIT_H
#define VKG_RAYTRACING_HIT_RCHIT_H

#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "../resources.h"
#include "../../sampling.h"
#include "../../brdf.h"
#include "../../punctual_light.h"
#ifdef USE_ATMOSPHERE
  #include "../../atmosphere/lighting.h"
#endif

layout(location = 0) rayPayloadInNV RayPayload prd;
layout(location = 1) rayPayloadNV ShadowRayPayload prdShadow;

#include "../trace.h"

hitAttributeNV vec2 hit;
#include "../vertex.h"

float evaluateFresnelDielectric(float et, vec3 N, vec3 rayDir) {
  const float NdotD = dot(N, rayDir);
  if(NdotD > 0.0) et = 1.0 / et;
  const float cosi = abs(NdotD);
  float sint = 1.0f - cosi * cosi;
  sint = (0.0f < sint) ? sqrt(sint) / et : 0.0f;
  if(sint > 1.0) return 1.0;

  // Handle total internal reflection.
  if(1.0f < sint) { return 1.0f; }

  float cost = 1.0f - sint * sint;
  cost = (0.0f < cost) ? sqrt(cost) : 0.0f;

  const float et_cosi = et * cosi;
  const float et_cost = et * cost;

  const float rPerpendicular = (cosi - et_cost) / (cosi + et_cost);
  const float rParallel = (et_cosi - cost) / (et_cosi + cost);

  const float result = (rParallel * rParallel + rPerpendicular * rPerpendicular) * 0.5f;

  return (result <= 1.0f) ? result : 1.0f;
}

bool shadowTrace(vec3 origin, vec3 lightDir, float maxDistance) {
  RayDesc ray;
  ray.origin = origin;
  ray.direction = lightDir;
  ray.tmin = 0.0;
  ray.tmax = maxDistance;
  ray.rayFlags = gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV |
                 gl_RayFlagsSkipClosestHitShaderNV;
  ray.cullMask = opaqueMask;
  ray.sbtRecordOffset = 0; //because we don't have to execute any shaders
  ray.sbtRecordStride = 0;
  ray.missIndex = 1;
  prdShadow.shadowed = true;
  trace(tlas, ray, 1);
  return prdShadow.shadowed;
}

void main() {
  MeshInstanceDesc ins = meshInstances[gl_InstanceCustomIndexNV];

  PrimitiveDesc primitive =
    primitives[ins.primitive + clamp(frame, 0, ins.primitiveCount - 1)];
  MaterialDesc material =
    materials[ins.material + clamp(frame, 0, ins.materialCount - 1)];

  VertexState state;
  MaterialInfo materialInfo;
  getShadingState(primitive, material, state, materialInfo);

  prd.origin = state.pos;

#ifdef SHADING_UNLIT
  prd.radiance = prd.attenuation * materialInfo.diffuseColor;
  prd.done = 1;
  return;
#endif

  vec3 color = vec3(0.0, 0.0, 0.0);
  vec3 viewDir = normalize(gl_WorldRayOriginNV - state.pos);
  for(int i = 0; i < lighting.numLights; ++i) {
    LightDesc light = lights[i];
    const float maxDis = length(light.location - state.pos);
    vec3 rayDir = normalize(light.location - state.pos);
    vec3 origin = offsetRayUE5(state.pos, state.geom_normal, rayDir);
    bool shadowed = shadowTrace(origin, rayDir, maxDis);

    vec3 directLight = vec3(0);

    vec3 F = vec3(0);
    directLight = brdf(rayDir, materialInfo, state.normal, viewDir, F) *
                  pointLightRadiance(light, state.pos, state.normal);
    color += directLight * (shadowed ? 0.01 : 1);
  }

#ifdef USE_ATMOSPHERE
  {
    vec3 rayDir = sun_direction.xyz;
    vec3 origin = offsetRayUE5(state.pos, state.geom_normal, rayDir);
    bool shadowed = shadowTrace(origin, rayDir, 1e9);

    vec3 directLight = vec3(0);

    vec3 F = vec3(0);
    directLight = brdf(rayDir, materialInfo, state.normal, viewDir, F) *
                  atmosphereLight(state.pos, state.normal, gl_WorldRayOriginNV);
    color += directLight * (shadowed ? 0.01 : 1);
  }
#endif

  color = color * materialInfo.ao;
  color += materialInfo.emissive;

  color *= prd.attenuation;
  prd.radiance += color;

#ifdef SHADING_REFLECTIVE
  prd.direction = reflect(gl_WorldRayDirectionNV, state.normal);
  prd.origin = offsetRayUE5(state.pos, state.geom_normal, prd.direction);
  prd.attenuation *= materialInfo.specularColor; //TODO may not be right
  prd.done = 0;
#else
  prd.done = 1;
#endif

#ifdef SHADING_REFRACTIVE
#endif
}
#endif
