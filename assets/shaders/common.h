#ifndef VKG_COMMON_H
#define VKG_COMMON_H

const uint nullIdx = ~0u;

const uint MaterialType_BRDF = 0x1u;
const uint MaterialType_BRDFSG = 0x2u;
const uint MaterialType_Reflective = 0x4u;
const uint MaterialType_Refractive = 0x8u;
const uint MaterialType_None = 0x10u;
const uint MaterialType_Transparent = 0x20u;
const uint MaterialType_Terrain = 0x40u;
struct MaterialUBO {
  vec4 baseColorFactor; // a: refractive,
  vec4 pbrFactor;       //  g: roughness, b: metallic, a: ?
  vec4 emissiveFactor;
  float occlusionStrength;
  float alphaCutoff;
  uint colorTex, pbrTex, normalTex, occlusionTex, emissiveTex, heightTex;
  uint type;
};

struct MeshInstanceUBO {
  uint material;
  uint primitive;
  uint node;
  uint instance;
  bool visible;
  uint drawGroupID;
};

struct AABB {
  vec3 min, max;
};

struct UIntRange {
  uint start;
  uint size;
};

struct PrimitiveUBO {
  UIntRange index, position, normal, uv;
  AABB aabb;
};

struct Vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;
};

struct LightUBO {
  uint numLights;
  float exposure, gamma;
};

struct LightInstanceUBO {
  vec3 color;
  float intensity;
  vec3 location;
  float range;
};

struct CameraUBO {
  mat4 view;
  mat4 proj;
  mat4 projView;
  mat4 invProjView;
  vec4 eye;
  vec4 r, v;
  float w, h, fov;
  float zNear, zFar;
  uint frame;
};

struct Transform {
  vec3 translation;
  vec3 scale;
  vec4 rotation;
};

mat4 toMatrix(Transform t) {
  float qxx = t.rotation.x * t.rotation.x;
  float qyy = t.rotation.y * t.rotation.y;
  float qzz = t.rotation.z * t.rotation.z;
  float qxz = t.rotation.x * t.rotation.z;
  float qxy = t.rotation.x * t.rotation.y;
  float qyz = t.rotation.y * t.rotation.z;
  float qwx = t.rotation.w * t.rotation.x;
  float qwy = t.rotation.w * t.rotation.y;
  float qwz = t.rotation.w * t.rotation.z;
  // clang-format off
  mat4 model = mat4(
    t.scale.x*(1-2*(qyy+qzz)), t.scale.x*(2*(qxy + qwz))  ,t.scale.x*(2*(qxz-qwy))  , 0,
    t.scale.y*(2*(qxy-qwz))  , t.scale.y*(1-2*(qxx + qzz)),t.scale.y*(2*(qyz+qwx))  , 0,
    t.scale.z*(2*(qxz+qwy))  , t.scale.z*(2*(qyz-qwx))    ,t.scale.z*(1-2*(qxx+qyy)), 0,
    0                   , 0                     ,0                   , 1);
  // clang-format on
  model[3] = vec4(t.translation.x, t.translation.y, t.translation.z, 1.0);
  return model;
}

struct VkDrawIndexedIndirectCommand {
  uint indexCount;
  uint instanceCount;
  uint firstIndex;
  int vertexOffset;
  uint firstInstance;
};

struct Frustum {
  vec4 planes[6];
};
#endif //VKG_COMMON_H
