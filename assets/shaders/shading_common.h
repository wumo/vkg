#ifndef VKG_SHADING_COMMON_H
#define VKG_SHADING_COMMON_H

vec3 RGBToYCoCg(vec3 RGB) {
  float Y = dot(RGB, vec3(1, 2, 1)) * 0.25;
  float Co = dot(RGB, vec3(2, 0, -2)) * 0.25 + (0.5 * 256.0 / 255.0);
  float Cg = dot(RGB, vec3(-1, 2, -1)) * 0.25 + (0.5 * 256.0 / 255.0);

  vec3 YCoCg = vec3(Y, Co, Cg);
  return YCoCg;
}

vec3 YCoCgToRGB(vec3 YCoCg) {
  float Y = YCoCg.x;
  float Co = YCoCg.y - (0.5 * 256.0 / 255.0);
  float Cg = YCoCg.z - (0.5 * 256.0 / 255.0);

  float R = Y + Co - Cg;
  float G = Y + Cg;
  float B = Y - Co - Cg;

  vec3 RGB = vec3(R, G, B);
  return RGB;
}

#endif //VKG_SHADING_COMMON_H
