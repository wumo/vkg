#ifndef VKG_DEFERRED_LIGHTING_H
#define VKG_DEFERRED_LIGHTING_H

#include "../math.h"
#define SUBPASS
#include "resources.h"
#include "../tonemap.h"
#ifdef USE_ATMOSPHERE
  #include "../atmosphere/lighting.h"
#endif
#include "../brdf.h"
#include "../punctual_light.h"
#ifdef USE_SHADOW_MAP
  #include "csm/csm.h"
#endif

layout(location = 0) out vec4 outColor;

struct PointInfo {
  vec3 position;
  vec3 normal;
  vec3 diffuseColor;
  float ao;
  vec3 specularColor;
  float perceptualRoughness;
  vec3 emissive;
  float depth;
  bool useSky;
};

vec3 view_ray(CameraUBO cam) {
  vec3 origin = vec3(cam.eye);
  float a = cam.w / cam.h;
  float f = tan(cam.fov / 2);
  vec3 r = cam.zNear * a * f * cam.r.xyz;
  vec3 u = -f * cam.zNear * normalize(cross(cam.r.xyz, cam.v.xyz));
  vec3 v = cam.zNear * cam.v.xyz;

  vec2 c = gl_FragCoord.xy / vec2(cam.w, cam.h) * 2.0 - 1.0;
  return normalize(v + c.x * r + c.y * u);
}

vec3 shadeBackground() {
#ifdef USE_ATMOSPHERE
  vec3 view_direction = view_ray(cam);
  return skyBackground(cam.eye.xyz, view_direction);
#else
  return vec3(0);
#endif
}

PointInfo unpack() {
  PointInfo p;
  p.position = SUBPASS_LOAD(samplerPosition).rgb;
  vec4 normalA = SUBPASS_LOAD(samplerNormal).rgba;
  p.normal = normalA.xyz;
  p.useSky = normalA.a > 0.5;
  vec4 diffuseAO = SUBPASS_LOAD(samplerDiffuse);
  p.diffuseColor = diffuseAO.rgb;
  p.ao = diffuseAO.w;
  vec4 specularRoughness = SUBPASS_LOAD(samplerSpecular);
  p.specularColor = specularRoughness.rgb;
  p.perceptualRoughness = specularRoughness.w;
  p.emissive = SUBPASS_LOAD(samplerEmissive).rgb;
  p.depth = SUBPASS_LOAD(samplerDepth).r;
  return p;
}

void main() {
  PointInfo p = unpack();
  if(p.depth == 1) { //
    outColor.rgb = LINEARtoSRGB(shadeBackground());
  } else {
    if(isZero(p.normal)) {
      outColor.rgb = LINEARtoSRGB(p.diffuseColor);
      return;
    }
    p.perceptualRoughness = clamp(p.perceptualRoughness, 0.0, 1.0);
    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    float alphaRoughness = p.perceptualRoughness * p.perceptualRoughness;

    // Compute reflectance.
    float reflectance = max(max(p.specularColor.r, p.specularColor.g), p.specularColor.b);

    vec3 specularEnvironmentR0 = p.specularColor.rgb;
    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));

    MaterialInfo materialInfo = MaterialInfo(
      p.perceptualRoughness, alphaRoughness, p.diffuseColor, p.specularColor,
      specularEnvironmentR0, specularEnvironmentR90, p.ao, p.emissive, 1.0);

    // LIGHTING
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 view = normalize(cam.eye.xyz - p.position);

    vec3 F = vec3(0);
    for(int i = 0; i < lighting.numLights; ++i) {
      LightInstanceUBO light = lights[i];
      vec3 rayDir = normalize(light.location - p.position);
      color += brdf(rayDir, materialInfo, p.normal, view, F) *
               pointLightRadiance(light, p.position, p.normal);
    }

#ifdef USE_ATMOSPHERE
    if(p.useSky) {
      vec3 rayDir = sun_direction.xyz;
  #ifdef USE_SHADOW_MAP
      bool shadowed = shadowTrace(p.position);
  #else
      bool shadowed = false;
  #endif
      vec3 directLight = brdf(rayDir, materialInfo, p.normal, view, F) *
                         atmosphereLight(p.position, p.normal, cam.eye.xyz);
      color += directLight * (shadowed ? 0.1 : 1);
    }
#endif

    color = color * p.ao;
    color += p.emissive;

//    outColor.rgb = LINEARtoSRGB(color);
    outColor.rgb = vec3(1);

    //    uint cIdx = cascadeIndex(p.position);
    //    switch(cIdx) {
    //      case 0: outColor.rgb = vec3(1, 0, 0); break;
    //      case 1: outColor.rgb = vec3(0, 1, 0); break;
    //      case 2: outColor.rgb = vec3(0, 0, 1); break;
    //      case 3: outColor.rgb = vec3(1, 1, 0); break;
    //      default: outColor.rgb = vec3(1, 1, 1); break;
    //    }
    //
    //    if(gl_FragCoord.x > cam.w / 2) {
    //      float sZ = shadowCoordDepth(p.position);
    //      outColor.rgb = vec3(sZ, sZ, sZ);
    //    } else {
    //      float smZ = shadowMapDepth(p.position);
    //      outColor.rgb = vec3(smZ, smZ, smZ);
    //    }
  }
}

#endif //VKG_LIGHTING_H
