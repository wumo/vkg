#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "resources.h"
#include "../sampling.h"
#include "../brdf.h"
#include "../punctual_light.h"

layout(location = 0) rayPayloadInNV RayPayload prd;
layout(location = 2) rayPayloadInNV ShadowRayPayload prdShadow;

hitAttributeNV vec2 hit;

// #include "trace.h"
#include "vertex.h"

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

void main() {

  MeshInstanceUBO ins = meshInstances[gl_InstanceCustomIndexNV];

  PrimitiveUBO primitive =
    primitives[ins.primitive + clamp(frame, 0, ins.primitiveCount - 1)];
  MaterialUBO material = materials[ins.material + clamp(frame, 0, ins.materialCount - 1)];

  VertexState state;
  MaterialInfo materialInfo;
  getShadingState(primitive, material, state, materialInfo);

  if(material.type == MaterialType_None) {
    prd.radiance = materialInfo.diffuseColor;
    prd.done = 1;
    return;
  }

  prd.radiance = materialInfo.diffuseColor;
  prd.done = 1;
  return;

  //   vec3 color = vec3(0.0, 0.0, 0.0);
  //   vec3 viewDir = normalize(gl_WorldRayOriginNV - state.pos);
  //   for(int i = 0; i < lighting.numLights; ++i) {
  //     LightInstanceUBO light = lights[i];
  //     const float maxDis = length(light.location - state.pos);
  //     vec3 rayDir = normalize(light.location - state.pos);
  //     vec3 origin = offsetRayUE5(state.pos, state.geom_normal, rayDir);
  //     bool shadowed = shadowTrace(origin, rayDir, maxDis);

  //     vec3 directLight = vec3(0);
  //     vec3 ambientLight = vec3(0);
  //     vec3 reflectLight = vec3(0);
  //     vec3 refractLight = vec3(0);

  //     vec3 F = vec3(0);
  //     directLight = brdf(rayDir, materialInfo, state.normal, viewDir, F) *
  //                   pointLightRadiance(light, state.pos, state.normal);
  // #ifdef AMBIENT_TRACE
  //     // vec3 ambientLight = reflectTrace(state.pos, gl_WorldRayDirectionNV, state.normal);
  //     ambientLight *= materialInfo.diffuseColor * (1 - F);
  // #endif
  // #ifdef REFLECT_TRACE
  //     reflectLight = reflectTrace(origin, gl_WorldRayDirectionNV, state.normal);
  //     reflectLight *= materialInfo.specularColor * F;
  // #endif
  //     color += (directLight + ambientLight + reflectLight + refractLight) *
  //              (shadowed ? 0.1 : 1); //TODO reflect term maybe not right
  //   }

  // #ifdef USE_ATMOSPHERE
  //   vec3 rayDir = sun_direction.xyz;
  //   vec3 origin = offsetRayUE5(state.pos, state.geom_normal, rayDir);
  //   bool shadowed = shadowTrace(origin, rayDir, 1e9);

  //   vec3 directLight = vec3(0);
  //   vec3 ambientLight = vec3(0);
  //   vec3 reflectLight = vec3(0);
  //   vec3 refractLight = vec3(0);

  //   vec3 F = vec3(0);
  //   directLight = brdf(rayDir, materialInfo, state.normal, viewDir, F) *
  //                 atmosphereLight(state.pos, state.normal, gl_WorldRayOriginNV);
  //   #ifdef REFLECT_TRACE
  //   reflectLight = reflectTrace(origin, gl_WorldRayDirectionNV, state.normal);
  //   reflectLight *= materialInfo.specularColor * F;
  //   #endif
  //   color += (directLight + ambientLight + reflectLight + refractLight) *
  //            (shadowed ? 0.1 : 1); //TODO reflect term maybe not right
  // #endif

  //   color = color * materialInfo.ao;
  //   color += materialInfo.emissive;

  //   prd.color = color;
  //   prd.hitT = gl_HitTNV;
}
