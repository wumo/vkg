#ifndef VKG_PATHTRACING_LIGHTING_H
#define VKG_PATHTRACING_LIGHTING_H

#include "../sampling.h"
#include "resources.h"
#ifdef USE_ATMOSPHERE
#include "../atmosphere/lighting.h"
#endif
#include "../brdf.h"
#include "../punctual_light.h"

layout(location = 0) rayPayloadInNV PathTracingRayPayload prd;
layout(location = 2) rayPayloadInNV PathTracingShadowRayPayload prdShadow;

hitAttributeNV vec2 hit;

#include "../raytracing/vertex.h"

void main() {
  MeshInstanceUBO instance = meshInstances[gl_InstanceCustomIndexNV];
  PrimitiveUBO primitive = primitives[instance.primitive];
  MaterialUBO material = materials[instance.material];
  
  VertexState state;
  MaterialInfo materialInfo;
  getShadingState(primitive, material, state, materialInfo);
  
  prd.hitT = gl_HitTNV;
  if(material.type == MaterialType_None) {
    prd.flags = FLAG_DONE;
    prd.weight = vec3(1.0);
    prd.contribution = materialInfo.diffuseColor;
    return;
  }
  if(has_flag(prd.flags, FLAG_PRIMARY_TEST)) {
    prd.contribution = materialInfo.diffuseColor + materialInfo.emissive;
    prd.rayDir = state.normal;
    return;
  }
  
  prd.contribution = vec3(0.0, 0.0, 0.0);
  vec3 contribution = vec3(0.0, 0.0, 0.0);
  vec3 viewDir = normalize(gl_WorldRayOriginNV - state.pos);
  
  vec3 result = vec3(0, 0, 0);
#ifdef USE_ATMOSPHERE
  uint lightIdx = uint(rnd(prd.seed) * (lighting.numLights + 1));
#else
  uint lightIdx = uint(rnd(prd.seed) * lighting.numLights);
#endif
  vec3 to_light;
  float lightDist;
  vec3 F = vec3(0);

#ifdef USE_ATMOSPHERE
  if(lightIdx == lighting.numLights) { // atmosphere
    to_light = sun_direction.xyz;
    lightDist = 1e9;
    contribution += prd.weight * brdf(to_light, materialInfo, state.normal, viewDir, F) *
                    atmosphereLight(state.pos, state.normal, gl_WorldRayOriginNV);
  } else {
#endif
  LightInstanceUBO light = lights[lightIdx];
  to_light = light.location - state.pos;
  lightDist = length(to_light);
  to_light = normalize(to_light);
  
  contribution += prd.weight * brdf(to_light, materialInfo, state.normal, viewDir, F) *
                  pointLightRadiance(light, state.pos, state.normal);
#ifdef USE_ATMOSPHERE
  }
#endif
  
  contribution = contribution * materialInfo.ao;
  
  if(debugMode == 0) {
    // New Ray  (origin)
    prd.rayOrigin = offsetRay(state.pos, state.geom_normal);
    
    // New direction and reflectance
    vec3 reflectance = vec3(0);
    vec3 rayDir;
    AngularInfo angularInfo = getAngularInfo(to_light, state.normal, viewDir);
    float F = specularReflection(materialInfo, angularInfo).x;
    float e = rnd(prd.seed);
    if(e > F) {
      vec3 T, B, N;
      N = state.normal;
      createCoordinateSystem(N, T, B);
      // Randomly sample the hemisphere
      prd.rayDir = samplingHemisphere(prd.seed, T, B, N);
      prd.weight *= materialInfo.diffuseColor.rgb;
    } else {
      vec3 V = viewDir;
      vec3 N = state.normal;
      // Randomly sample the NDF to get a microfacet in our BRDF to reflect off
      vec3 H = getGGXMicrofacet(rnd2(prd.seed), N, materialInfo.perceptualRoughness);
      // Compute the outgoing direction based on this (perfectly reflective) microfacet
      prd.rayDir = normalize(2.f * dot(V, H) * H - V);
      prd.weight *= materialInfo.specularColor;
    }
  } else {
    prd.flags = FLAG_DONE;
    prd.weight = vec3(1.0);
    prd.contribution = vec3(0);
    
    return;
  }
  
  // Add contribution from next event estimation if not shadowed
  //---------------------------------------------------------------------------------------------
  
  // cast a shadow ray; assuming light is always outside
  vec3 direction = normalize(to_light);
  vec3 origin = offsetRayUE5(state.pos, state.geom_normal, direction);
  
  // prepare the ray and payload but trace at the end to reduce the amount of data that has
  // to be recovered after coming back from the shadow trace
  prdShadow.isHit = true;
  prdShadow.seed = 0;
  uint rayFlags = gl_RayFlagsOpaqueNV | gl_RayFlagsTerminateOnFirstHitNV |
                  gl_RayFlagsSkipClosestHitShaderNV;
  
  traceNV(
    Scene,     // acceleration structure
    rayFlags,  // rayFlags
    0xFF,      // cullMask
    1,         // sbtRecordOffset
    1,         // sbtRecordStride
    1,         // missIndex
    origin,    // ray origin
    1e-3,      // ray min range
    direction, // ray direction
    lightDist, // ray max range
    2          // payload layout(location = 1)
         );
  
  // add to ray contribution from next event estiation
  if(!prdShadow.isHit) prd.contribution += contribution;
  // if put this before traceNV, prd.contribution will be wrong.
  prd.contribution += materialInfo.emissive;
}

#endif //VKG_PATHTRACING_LIGHTING_H
