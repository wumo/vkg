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
  //  vec2 ds, dt; //ray diff
  float lambda; //ray Cone
};

//refer to "Texture Level of Detail Strategies for Real-Time Ray Tracing"
//void rayDiff(
//  inout vec2 ds, inout vec2 dt, vec3 p0, vec3 p1, vec3 p2, vec2 T0, vec2 T1, vec2 T2) {
//  vec3 d = normalize(prd.direction); //TODO the paper may be wrong?
//  float t = gl_HitTNV;
//  vec3 e1 = vec3(gl_ObjectToWorldNV * vec4(p1 - p0, 1.0));
//  vec3 e2 = vec3(gl_ObjectToWorldNV * vec4(p2 - p0, 1.0));
//  vec3 cu = cross(e2, d);
//  vec3 cv = cross(d, e1);
//  vec3 q = prd.rayDiff.dOdx + t * prd.rayDiff.dDdx;
//  vec3 r = prd.rayDiff.dOdy + t * prd.rayDiff.dDdy;
//  float k = dot(cross(e1, e2), d);
//  vec2 du = vec2(dot(cu, q) / k, dot(cu, r) / k);
//  vec2 dv = vec2(dot(cv, q) / k, dot(cv, r) / k);
//
//  vec2 g1 = T1 - T0;
//  vec2 g2 = T2 - T0;
//  ds = vec2(du.x * g1.x + dv.x * g2.x, du.y * g1.x + dv.y * g2.x);
//  dt = vec2(du.x * g1.y + dv.x * g2.y, du.y * g1.y + dv.y * g2.y);
//}

//see "Texture Level of Detail Strategies for Real-Time Ray Tracing"
float rayCone(vec3 p0, vec3 p1, vec3 p2, vec2 T0, vec2 T1, vec2 T2, vec3 normal) {
  //propagate
  prd.rayCone.width = prd.rayCone.spreadAngle * gl_HitTNV + prd.rayCone.width;
  //prd.rayCone.spreadAngle will not change;

  //base lod
  float T_a = abs((T1.x - T0.x) * (T2.y - T0.y) - (T2.x - T0.x) * (T1.y - T0.y));
  //  float P_a = abs((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
  vec3 e1 = vec3(gl_ObjectToWorldNV * vec4(p1 - p0, 1));
  vec3 e2 = vec3(gl_ObjectToWorldNV * vec4(p2 - p0, 1));
  float P_a = length(cross(e1, e2));

  float lambda = 0.5 * log2(T_a / P_a);
  lambda += log2(abs(prd.rayCone.width));
  lambda -= log2(abs(dot(gl_WorldRayDirectionNV, normal)));
  return lambda;
}

float texLodRayCone(uint texId, float lambda) {
  int levels = textureQueryLevels(textures[texId]);
  ivec2 size = textureSize(textures[texId], 0);
  float lod = lambda + 0.5 * log2(float(size.x * size.y));
  return clamp(lod, 0, levels - 1.0);
}

//float texLodRayDiff(uint texId, vec2 ds, vec2 dt) {
//  int levels = textureQueryLevels(textures[texId]);
//  ivec2 size = textureSize(textures[texId], 0);
//  float w = size.x;
//  float h = size.y;
//
//  float p = max(abs(w * ds.x) + abs(w * ds.y), abs(h * dt.x) + abs(h * dt.y));
//
//  //  float p = max(
//  //    sqrt(w * w * ds.x * ds.x + h * h * dt.x * dt.x),
//  //    sqrt(w * w * ds.y * ds.y + h * h * dt.y * dt.y));
//
//  float lambda = isZero(p) ? 0 : log2(ceil(p));
//  float lod = clamp(lambda * 0.5, 0, levels - 1);
//  return lod;
//}

vec4 texLod(uint texId, vec2 coord, float lambda) {
  float lod = texLodRayCone(texId, lambda);
  return textureLod(textures[texId], coord, lod);
}

void getVertexState(
  in PrimitiveDesc primitive, in MaterialDesc material, inout VertexState state) {
  const vec3 barycentrics = vec3(1.0f - hit.x - hit.y, hit.x, hit.y);

  const uint faceIndex = gl_PrimitiveID;
  const uint indexOffset = primitive.index.start + 3 * faceIndex;
  const uint i0 = indices[indexOffset];
  const uint i1 = indices[indexOffset + 1];
  const uint i2 = indices[indexOffset + 2];
  const vec3 pos0 = positions[primitive.position.start + i0];
  const vec3 pos1 = positions[primitive.position.start + i1];
  const vec3 pos2 = positions[primitive.position.start + i2];
  const vec3 position = BaryLerp(pos0, pos1, pos2, barycentrics);
  const vec3 world_position = vec3(gl_ObjectToWorldNV * vec4(position, 1.0));

  const vec3 n0 = normals[primitive.normal.start + i0];
  const vec3 n1 = normals[primitive.normal.start + i1];
  const vec3 n2 = normals[primitive.normal.start + i2];
  const vec3 normal = normalize(BaryLerp(n0, n1, n2, barycentrics));
  const vec3 world_normal = normalize(vec3(normal * gl_WorldToObjectNV));
  const vec3 geom_normal = normalize(cross(pos1 - pos0, pos2 - pos0));
  vec3 world_geom_normal = normalize(vec3(geom_normal * gl_WorldToObjectNV));

  // flip geometry normal to the side of the incident ray
  if(dot(world_geom_normal, gl_WorldRayDirectionNV) > 0.0) world_geom_normal *= -1.0f;

  const vec2 uv0 = uvs[primitive.uv.start + i0];
  const vec2 uv1 = uvs[primitive.uv.start + i1];
  const vec2 uv2 = uvs[primitive.uv.start + i2];
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

  //  rayDiff(state.ds, state.dt, pos0, pos1, pos2, uv0, uv1, uv2);
  state.lambda = rayCone(pos0, pos1, pos2, uv0, uv1, uv2, state.geom_normal);
}

void getMaterialInfo(
  in MaterialDesc material, in VertexState vertexState, inout MaterialInfo materialInfo) {
  vec2 texcoord0 = vertexState.texcoord0;
  float lambda = vertexState.lambda;

  vec4 baseColor = material.colorTex != nullIdx ?
                     SRGBtoLINEAR4(texLod(material.colorTex, texcoord0, lambda)) :
                     vec4(1, 1, 1, 1);
  baseColor = material.baseColorFactor * baseColor;

  //  materialInfo.diffuseColor = baseColor.rgb;
  //  if(material.colorTex != nullIdx) {
  //    float lod = texLodRayCone(material.colorTex, lambda);
  //
  //    if(lod <= 1) materialInfo.diffuseColor = vec3(1, 0, 0);
  //    else if(lod < 2)
  //      materialInfo.diffuseColor = vec3(0, 1, 0);
  //    else if(lod < 3)
  //      materialInfo.diffuseColor = vec3(0, 0, 1);
  //    else if(lod < 4)
  //      materialInfo.diffuseColor = vec3(1, 1, 0);
  //    else if(lod < 5)
  //      materialInfo.diffuseColor = vec3(0, 1, 1);
  //    else if(lod < 6)
  //      materialInfo.diffuseColor = vec3(1, 0, 1);
  //    else
  //      materialInfo.diffuseColor = vec3(lod / 10.0);
  //  }
  //  return;

  if(material.type == MaterialType_None) {
    materialInfo.diffuseColor = baseColor.rgb;
    return;
  } else if(material.type == MaterialType_Transparent) {
    materialInfo.diffuseColor = baseColor.rgb;
    materialInfo.eta = baseColor.a;
    return;
  }

  float perceptualRoughness = 0.0;
  vec3 diffuseColor = vec3(0);
  vec3 specularColor = vec3(0.0);

  if(material.type == MaterialType_BRDFSG) {
    vec4 specular = material.pbrTex != nullIdx ?
                      SRGBtoLINEAR4(texLod(material.pbrTex, texcoord0, lambda)) :
                      vec4(1, 1, 1, 1);
    vec3 f0 = specular.rgb * material.pbrFactor.rgb;
    specularColor = f0;
    float oneMinusSpecularStrength = 1.0 - max(max(f0.r, f0.g), f0.b);
    perceptualRoughness =
      (1.0 - specular.w * material.pbrFactor.w); // glossiness to roughness
    diffuseColor = baseColor.rgb * oneMinusSpecularStrength;
    //    float metallic=solveMetallic(diffuseColor,specularColor,oneMinusSpecularStrength);
  } else {
    vec2 pbr = material.pbrTex != nullIdx ?
                 texLod(material.pbrTex, texcoord0, lambda).gb :
                 vec2(1, 1);
    pbr = material.pbrFactor.gb * pbr;
    perceptualRoughness = pbr.x;
    float metallic = pbr.y;
    vec3 f0 = vec3(0.04);
    diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
    specularColor = mix(f0, baseColor.rgb, metallic);
  }

  float ao = material.occlusionTex != nullIdx ?
               texLod(material.occlusionTex, texcoord0, lambda).r :
               1;
  ao = mix(1, ao, material.occlusionStrength);

  vec3 emissive = material.emissiveTex != nullIdx ?
                    SRGBtoLINEAR4(texLod(material.emissiveTex, texcoord0, lambda)).rgb :
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
  in PrimitiveDesc primitive, in MaterialDesc material, out VertexState vertexState,
  out MaterialInfo materialInfo) {
  getVertexState(primitive, material, vertexState);
  getMaterialInfo(material, vertexState, materialInfo);
}

#endif //VKG_RT_VERTEX_H
