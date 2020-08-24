#version 450
#extension GL_GOOGLE_include_directive : require
#include "../tonemap.h"
#include "resources.h"
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in flat uint inMaterialID;

layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outSpecular;
layout(location = 3) out vec4 outEmissive;

vec3 computeNormal(vec3 sampledNormal) {
  vec3 pos_dx = dFdx(inWorldPos);
  vec3 pos_dy = dFdy(inWorldPos);
  vec3 tex_dx = dFdx(vec3(inUV0, 0.0));
  vec3 tex_dy = dFdy(vec3(inUV0, 0.0));
  vec3 t =
    (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
  vec3 ng = normalize(inNormal);
  t = normalize(t - ng * dot(ng, t));
  vec3 b = normalize(cross(ng, t));
  mat3 tbn = mat3(t, b, ng);
  return normalize(tbn * (2 * sampledNormal - 1));
}

void main() {
  MaterialUBO material = materials[inMaterialID];
  vec3 albedo = material.colorTex != nullIdx ?
                  SRGBtoLINEAR(texture(textures[material.colorTex], inUV0).rgb) :
                  vec3(1, 1, 1);
  albedo = material.baseColorFactor.rgb * albedo;
  vec3 normal;
  if(material.type == MaterialType_None) normal = vec3(0);
  else if(material.type == MaterialType_Terrain)
    normal = inNormal;
  else
    normal = material.normalTex != nullIdx ?
               computeNormal(texture(textures[material.normalTex], inUV0).rgb) :
               inNormal;

  vec3 diffuseColor;
  vec3 specularColor;
  float perceptualRoughness;
  if(material.type == MaterialType_BRDFSG) {
    vec4 specular = material.pbrTex != nullIdx ?
                      SRGBtoLINEAR4(texture(textures[material.pbrTex], inUV0)) :
                      vec4(1, 1, 1, 1);
    vec3 f0 = specular.rgb * material.pbrFactor.rgb;
    specularColor = f0;
    float oneMinusSpecularStrength = 1.0 - max(max(f0.r, f0.g), f0.b);
    perceptualRoughness =
      (1.0 - specular.w * material.pbrFactor.w); // glossiness to roughness
    diffuseColor = albedo * oneMinusSpecularStrength;
  } else {
    vec2 pbr = material.pbrTex != nullIdx ? texture(textures[material.pbrTex], inUV0).gb :
                                            vec2(1, 1);
    pbr = material.pbrFactor.gb * pbr;
    perceptualRoughness = pbr.x;
    float metallic = pbr.y;
    vec3 f0 = vec3(0.04);
    diffuseColor = albedo * (vec3(1.0) - f0) * (1.0 - metallic);
    specularColor = mix(f0, albedo, metallic);
  }
  float ao = material.occlusionTex != nullIdx ?
               texture(textures[material.occlusionTex], inUV0).r :
               1;
  ao = mix(1, ao, material.occlusionStrength);

  vec3 emissive = material.emissiveTex != nullIdx ?
                    SRGBtoLINEAR4(texture(textures[material.emissiveTex], inUV0)).rgb :
                    vec3(0, 0, 0);
  emissive = material.emissiveFactor.rgb * emissive;

  int applySky = 1;
  if(material.type == MaterialType_None) applySky = 0;
  outNormal = vec4(normal, applySky);
  outDiffuse = vec4(diffuseColor, ao);
  outSpecular = vec4(specularColor, perceptualRoughness);
  outEmissive.rgb = emissive;
}