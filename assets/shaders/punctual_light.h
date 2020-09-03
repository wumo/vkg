#ifndef VKG_PUNCTUAL_LIGHT_H
#define VKG_PUNCTUAL_LIGHT_H

#include "common.h"

// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#range-property
float getRangeAttenuation(float range, float distance) {
  if(range <= 0.0) {
    // negative range means unlimited
    return 1.0;
  }
  return max(min(1.0 - pow(distance / range, 4.0), 1.0), 0.0) / pow(distance, 2.0);
}

vec3 pointLightRadiance(LightDesc light, vec3 worldPos, vec3 worldNormal) {
  vec3 pointToLight = light.location - worldPos;
  float distance = length(pointToLight);
  float attenuation = light.range == 0 ? 1 : getRangeAttenuation(light.range, distance);
  return attenuation * light.intensity * light.color;
}

#endif //VKG_PUNCTUAL_LIGHT_H
