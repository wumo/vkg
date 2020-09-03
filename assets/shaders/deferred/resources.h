#ifndef VKG_DEFERRED_RESOURCES_H
#define VKG_DEFERRED_RESOURCES_H

#extension GL_EXT_scalar_block_layout : enable

#include "deferred_common.h"
layout(constant_id = 0) const uint maxNumTextures = 1;

layout(set = 0, binding = 0, scalar) readonly buffer Camera { CameraDesc camera; };
layout(set = 0, binding = 1, scalar) readonly buffer MeshesBuf {
  MeshInstanceDesc meshInstances[];
};
layout(set = 0, binding = 2, std430) readonly buffer TransformBuf { mat4 matrices[]; };
layout(set = 0, binding = 3, scalar) readonly buffer MaterialBuf {
  MaterialDesc materials[];
};
layout(set = 0, binding = 4) uniform sampler2D textures[maxNumTextures];

layout(set = 0, binding = 5) uniform LightingUBO { LightingDesc lighting; };
layout(set = 0, binding = 6, std430) readonly buffer LightsBuffer { LightDesc lights[]; };

#ifdef SUBPASS
  #ifdef MULTISAMPLE
    #define SUBPASS_INPUT subpassInputMS
    #define SUBPASS_LOAD(sampler) subpassLoad(sampler, gl_SampleID)
  #else
    #define SUBPASS_INPUT subpassInput
    #define SUBPASS_LOAD(sampler) subpassLoad(sampler)
  #endif

// clang-format off
layout(set = 1, binding = 0,input_attachment_index = 0) uniform SUBPASS_INPUT samplerNormal;
layout(set = 1, binding = 1,input_attachment_index = 1) uniform SUBPASS_INPUT samplerDiffuse;
layout(set = 1, binding = 2,input_attachment_index = 2) uniform SUBPASS_INPUT samplerSpecular;
layout(set = 1, binding = 3,input_attachment_index = 3) uniform SUBPASS_INPUT samplerEmissive;
layout(set = 1, binding = 4,input_attachment_index = 4) uniform SUBPASS_INPUT samplerDepth;
  // clang-format on
#endif

#ifdef USE_ATMOSPHERE
  #define ATMOSPHERE_SET 2
  #include "../atmosphere/resources.h"
#endif

#ifdef USE_SHADOW_MAP
layout(set = 3, binding = 0) uniform ShadowMapSettingUBO {
  ShadowMapSetting shadowMapSeting;
};
layout(set = 3, binding = 1) buffer Cascades { CascadeDesc cascades[]; };
layout(set = 3, binding = 2) uniform sampler2DArray shadowMap;
#endif

#endif //VKG_RESOURCES_H
