#ifndef VKG_TONEMAP_H
#define VKG_TONEMAP_H
const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 LINEARtoSRGB(vec3 color) { return pow(color, vec3(INV_GAMMA)); }

// sRGB to linear approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 SRGBtoLINEAR(vec3 srgbIn) { return pow(srgbIn, vec3(GAMMA)); }
vec4 SRGBtoLINEAR4(vec4 srgbIn) { return vec4(SRGBtoLINEAR(srgbIn.rgb), srgbIn.w); }

// Uncharted 2 tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 toneMapUncharted2Impl(vec3 color) {
  const float A = 0.15;
  const float B = 0.50;
  const float C = 0.10;
  const float D = 0.20;
  const float E = 0.02;
  const float F = 0.30;
  return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) -
         E / F;
}

vec3 toneMapUncharted(vec3 color) {
  const float W = 11.2;
  color = toneMapUncharted2Impl(color * 2.0);
  vec3 whiteScale = 1.0 / toneMapUncharted2Impl(vec3(W));
  return LINEARtoSRGB(color * whiteScale);
}

// Hejl Richard tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 toneMapHejlRichard(vec3 color) {
  color = max(vec3(0.0), color - vec3(0.004));
  return (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
}

// ACES tone map
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 toneMapACES(vec3 color) {
  const float A = 2.51;
  const float B = 0.03;
  const float C = 2.43;
  const float D = 0.59;
  const float E = 0.14;
  return LINEARtoSRGB(
    clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0));
}

#define TONEMAP_UNCHARTED

vec4 toneMap(vec4 color, float exposure) {
  color *= exposure;

#ifdef TONEMAP_UNCHARTED
  return vec4(toneMapUncharted(color.rgb), color.a);
#endif

#ifdef TONEMAP_HEJLRICHARD
  return vec4(toneMapHejlRichard(color.rgb), color.a);
#endif

#ifdef TONEMAP_ACES
  return vec4(toneMapACES(color.rgb), color.a);
#endif

  return vec4(LINEARtoSRGB(color.rgb), color.a);
}

#endif