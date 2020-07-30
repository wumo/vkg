#ifndef VKG_MATH_H
#define VKG_MATH_H

const float M_PI = 3.14159265358979323846;
#define PRECISION 0.000001
bool isZero(float val) {
  return step(-PRECISION, val) * (1.0 - step(PRECISION, val)) == 1.0;
}

bool isZero(vec3 v) { return isZero(v.x) && isZero(v.y) && isZero(v.z); }

vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 BaryLerp(vec4 a, vec4 b, vec4 c, vec3 barycentrics) {
  return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

#endif //VKG_MATH_H
