#ifndef VKG_RT_VERTEX_H
#define VKG_RT_VERTEX_H

#include "rt_common.h"
#include "../brdf.h"

struct VertexState {
  vec3 pos;
  vec3 normal;
  vec3 geom_normal;
  vec2 texcoord0;
  vec3 tangent;
  vec3 bitangent;
};

//vec3 getVertex(uint index) {
//  return vec3(
//    positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2]);
//}
//
//vec3 getNormal(uint index) {
//  return vec3(normals[3 * index + 0], normals[3 * index + 1], normals[3 * index + 2]);
//}

void getVertexState(
  in PrimitiveUBO primitive, in MaterialUBO material, inout VertexState state) {
  const vec3 barycentrics = vec3(1.0f - hit.x - hit.y, hit.x, hit.y);

  const uint faceIndex = gl_PrimitiveID;
  const uint indexOffset = primitive.index.x + 3 * faceIndex;
  const uint i0 = indices[indexOffset];
  const uint i1 = indices[indexOffset + 1];
  const uint i2 = indices[indexOffset + 2];
  const vec3 pos0 = positions[primitive.position.x + i0];
  const vec3 pos1 = positions[primitive.position.x + i1];
  const vec3 pos2 = positions[primitive.position.x + i2];
  const vec3 position = BaryLerp(pos0, pos1, pos2, barycentrics);
  const vec3 world_position = vec3(gl_ObjectToWorldNV * vec4(position, 1.0));

  const vec3 n0 = normals[primitive.normal.x + i0];
  const vec3 n1 = normals[primitive.normal.x + i1];
  const vec3 n2 = normals[primitive.normal.x + i2];
  const vec3 normal = normalize(BaryLerp(n0, n1, n2, barycentrics));
  const vec3 world_normal = normalize(vec3(normal * gl_WorldToObjectNV));
  const vec3 geom_normal = normalize(cross(pos1 - pos0, pos2 - pos0));
  vec3 world_geom_normal = normalize(vec3(geom_normal * gl_WorldToObjectNV));

  // flip geometry normal to the side of the incident ray
  if(dot(world_geom_normal, gl_WorldRayDirectionNV) > 0.0) world_geom_normal *= -1.0f;

  const vec2 uv0 = uvs[primitive.uv.x + i0];
  const vec2 uv1 = uvs[primitive.uv.x + i1];
  const vec2 uv2 = uvs[primitive.uv.x + i2];
  vec2 uv = BaryLerp(uv0, uv1, uv2, barycentrics);

  state.pos = world_position;
  state.normal = world_normal;
  state.geom_normal = world_geom_normal;
  state.texcoord0 = uv;

  if(material.normalTex != nullIdx) {
    vec3 t = cross(world_normal, vec3(0, 1, 0));
    if(isZero(t)) t = cross(world_normal, vec3(0, 0, 1));
    t = normalize(t);
    vec3 b = cross(world_normal, t);
    vec3 sampledNormal = texture(textures[material.normalTex], state.texcoord0).rgb;
    sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
    mat3 TBN = mat3(t, b, world_normal);
    vec3 pnormal = normalize(TBN * sampledNormal);
    state.normal = pnormal;
    state.tangent = t;
    state.bitangent = b;
  }
  // Move normal to same side as geometric normal
  if(dot(state.normal, state.geom_normal) <= 0) state.normal *= -1.0f;
}

void getMaterialInfo(
  in MaterialUBO material, in vec2 texcoord0, inout MaterialInfo materialInfo) {
  vec3 baseColor = material.colorTex != nullIdx ?
                     SRGBtoLINEAR(texture(textures[material.colorTex], texcoord0)).xyz :
                     vec3(1, 1, 1);
  baseColor = material.baseColorFactor.rgb * baseColor;

  if(material.type == MaterialType_None) {
    materialInfo.diffuseColor = baseColor;
    return;
  }

  float perceptualRoughness = 0.0;
  vec3 diffuseColor = vec3(0);
  vec3 specularColor = vec3(0.0);

  if(material.type == MaterialType_BRDFSG) {
    vec4 specular = material.pbrTex != nullIdx ?
                      SRGBtoLINEAR(texture(textures[material.pbrTex], texcoord0)) :
                      vec4(1, 1, 1, 1);
    vec3 f0 = specular.rgb * material.pbrFactor.rgb;
    specularColor = f0;
    float oneMinusSpecularStrength = 1.0 - max(max(f0.r, f0.g), f0.b);
    perceptualRoughness =
      (1.0 - specular.w * material.pbrFactor.w); // glossiness to roughness
    diffuseColor = baseColor * oneMinusSpecularStrength;
    //    float metallic=solveMetallic(diffuseColor,specularColor,oneMinusSpecularStrength);
  } else {
    vec2 pbr = material.pbrTex != nullIdx ?
                 texture(textures[material.pbrTex], texcoord0).gb :
                 vec2(1, 1);
    pbr = material.pbrFactor.gb * pbr;
    perceptualRoughness = pbr.x;
    float metallic = pbr.y;
    vec3 f0 = vec3(0.04);
    diffuseColor = baseColor * (vec3(1.0) - f0) * (1.0 - metallic);
    specularColor = mix(f0, baseColor, metallic);
  }

  float ao = material.occlusionTex != nullIdx ?
               texture(textures[material.occlusionTex], texcoord0).r :
               1;
  ao = mix(1, ao, material.occlusionStrength);

  vec3 emissive = material.emissiveTex != nullIdx ?
                    SRGBtoLINEAR(texture(textures[material.emissiveTex], texcoord0)).rgb :
                    vec3(0, 0, 0);
  emissive = material.emissiveFactor.rgb * emissive;

  float eta = material.baseColorFactor.a;

  perceptualRoughness = clamp(perceptualRoughness, 0.0, 1.0);
  // Roughness is authored as perceptual roughness; as is convention,
  // convert to material roughness by squaring the perceptual roughness [2].
  float alphaRoughness = perceptualRoughness * perceptualRoughness;

  // Compute reflectance.
  float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

  vec3 specularEnvironmentR0 = specularColor.rgb;
  // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
  vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));

  materialInfo = MaterialInfo(
    perceptualRoughness, alphaRoughness, diffuseColor, specularColor,
    specularEnvironmentR0, specularEnvironmentR90, ao, emissive, eta);
}

void getShadingState(
  in PrimitiveUBO primitive, in MaterialUBO material, out VertexState vertexState,
  out MaterialInfo materialInfo) {
  getVertexState(primitive, material, vertexState);
  getMaterialInfo(material, vertexState.texcoord0, materialInfo);
}

#endif //VKG_RT_VERTEX_H
