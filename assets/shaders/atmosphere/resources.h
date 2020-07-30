#ifndef VKG_ATMOSPHERE_RESOURCES_H
#define VKG_ATMOSPHERE_RESOURCES_H

#ifndef ATMOSPHERE_SET
  #error "Define ATMOSPHERE_SET"
#endif

#include "atmosphere.h"

layout(set = ATMOSPHERE_SET, binding = 1) uniform SunUniform {
  vec4 white_point;
  vec4 earth_center;
  vec4 sun_direction;
  vec2 sun_size;
  float exposure;
  float sun_intensity;
};
layout(set = ATMOSPHERE_SET, binding = 2) uniform sampler2D transmittanceTex;
layout(set = ATMOSPHERE_SET, binding = 3) uniform sampler3D scatteringTex;
layout(set = ATMOSPHERE_SET, binding = 4) uniform sampler2D irradianceTex;

#endif //VKG_ATMOSPHERE_RESOURCES_H
