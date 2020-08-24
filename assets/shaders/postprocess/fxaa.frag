#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : enable
// #extension GL_EXT_debug_printf : enable

#include "../tonemap.h"

// from https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Shaders/Private/Fxaa3_11.ush

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstant {
  // Choose the amount of sub-pixel aliasing removal.
  // This can effect sharpness.
  //   1.00 - upper limit (softer)
  //   0.75 - default amount of filtering
  //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
  //   0.25 - almost off
  //   0.00 - completely off
  float fxaaQualitySubpix;
  // The minimum amount of local contrast required to apply algorithm.
  //   0.333 - too little (faster)
  //   0.250 - low quality
  //   0.166 - default
  //   0.125 - high quality
  //   0.063 - overkill (slower)
  float fxaaQualityEdgeThreshold;
  // Trims the algorithm from processing darks.
  //   0.0833 - upper limit (default, the start of visible unfiltered edges)
  //   0.0625 - high quality (faster)
  //   0.0312 - visible limit (slower)
  float fxaaQualityEdgeThresholdMin;
};

layout(set = 0, binding = 0) uniform sampler2D colorImg;

#define FXAA_QUALITY_PRESET == 20
#define FXAA_QUALITY_PS 3
#define FXAA_QUALITY_P0 1.5
#define FXAA_QUALITY_P1 2.0
#define FXAA_QUALITY_P2 8.0
#define saturate(x) clamp(x, 0.0, 1.0)
#define texTop(t, p) texture(t, p)

vec4 texOff(vec2 pos, ivec2 offset, vec2 scale) {
  pos += offset * scale;
  return texture(colorImg, pos);
}
float luma(vec4 rgba) { return dot(rgba.rgb, vec3(0.299f, 0.587f, 0.114f)); }

void main() {
  vec2 posM = inUV;
  ivec2 size = textureSize(colorImg, 0);
  vec2 offsetScale = vec2(1.0 / size.x, 1.0 / size.y);

  vec4 rgbM = texOff(posM, ivec2(0, 0), offsetScale);
  float lumaM = luma(rgbM);
  float lumaS = luma(texOff(posM, ivec2(0, 1), offsetScale));
  float lumaE = luma(texOff(posM, ivec2(1, 0), offsetScale));
  float lumaN = luma(texOff(posM, ivec2(0, -1), offsetScale));
  float lumaW = luma(texOff(posM, ivec2(-1, 0), offsetScale));

  float maxSM = max(lumaS, lumaM);
  float minSM = min(lumaS, lumaM);
  float maxESM = max(lumaE, maxSM);
  float minESM = min(lumaE, minSM);
  float maxWN = max(lumaN, lumaW);
  float minWN = min(lumaN, lumaW);
  float rangeMax = max(maxWN, maxESM);
  float rangeMin = min(minWN, minESM);
  float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
  float range = rangeMax - rangeMin;
  float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
  bool earlyExit = range < rangeMaxClamped;
  /*--------------------------------------------------------------------------*/
  if(earlyExit) {
    fragColor = rgbM;
    return;
  }
  /*--------------------------------------------------------------------------*/
  float lumaNW = luma(texOff(posM, ivec2(-1, -1), offsetScale));
  float lumaSE = luma(texOff(posM, ivec2(1, 1), offsetScale));
  float lumaNE = luma(texOff(posM, ivec2(1, -1), offsetScale));
  float lumaSW = luma(texOff(posM, ivec2(-1, 1), offsetScale));

  /*--------------------------------------------------------------------------*/
  float lumaNS = lumaN + lumaS;
  float lumaWE = lumaW + lumaE;
  float subpixRcpRange = 1.0 / range;
  float subpixNSWE = lumaNS + lumaWE;
  float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
  float edgeVert1 = (-2.0 * lumaM) + lumaWE;
  /*--------------------------------------------------------------------------*/
  float lumaNESE = lumaNE + lumaSE;
  float lumaNWNE = lumaNW + lumaNE;
  float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
  float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
  /*--------------------------------------------------------------------------*/
  float lumaNWSW = lumaNW + lumaSW;
  float lumaSWSE = lumaSW + lumaSE;
  float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
  float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
  float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
  float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
  float edgeHorz = abs(edgeHorz3) + edgeHorz4;
  float edgeVert = abs(edgeVert3) + edgeVert4;
  /*--------------------------------------------------------------------------*/
  float subpixNWSWNESE = lumaNWSW + lumaNESE;
  float lengthSign = offsetScale.x;
  bool horzSpan = edgeHorz >= edgeVert;
  float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
  /*--------------------------------------------------------------------------*/
  if(!horzSpan) lumaN = lumaW;
  if(!horzSpan) lumaS = lumaE;
  if(horzSpan) lengthSign = offsetScale.y;
  float subpixB = (subpixA * (1.0 / 12.0)) - lumaM;
  /*--------------------------------------------------------------------------*/
  float gradientN = lumaN - lumaM;
  float gradientS = lumaS - lumaM;
  float lumaNN = lumaN + lumaM;
  float lumaSS = lumaS + lumaM;
  bool pairN = abs(gradientN) >= abs(gradientS);
  float gradient = max(abs(gradientN), abs(gradientS));
  if(pairN) lengthSign = -lengthSign;
  float subpixC = saturate(abs(subpixB) * subpixRcpRange);
  /*--------------------------------------------------------------------------*/
  vec2 posB;
  posB.x = posM.x;
  posB.y = posM.y;
  vec2 offNP;
  offNP.x = (!horzSpan) ? 0.0 : offsetScale.x;
  offNP.y = (horzSpan) ? 0.0 : offsetScale.y;
  if(!horzSpan) posB.x += lengthSign * 0.5;
  if(horzSpan) posB.y += lengthSign * 0.5;
  /*--------------------------------------------------------------------------*/
  vec2 posN;
  posN.x = posB.x - offNP.x * FXAA_QUALITY_P0;
  posN.y = posB.y - offNP.y * FXAA_QUALITY_P0;
  vec2 posP;
  posP.x = posB.x + offNP.x * FXAA_QUALITY_P0;
  posP.y = posB.y + offNP.y * FXAA_QUALITY_P0;
  float subpixD = ((-2.0) * subpixC) + 3.0;
  float lumaEndN = luma(texTop(colorImg, posN));
  float subpixE = subpixC * subpixC;
  float lumaEndP = luma(texTop(colorImg, posP));
  /*--------------------------------------------------------------------------*/
  if(!pairN) lumaNN = lumaSS;
  float gradientScaled = gradient * 1.0 / 4.0;
  float lumaMM = lumaM - lumaNN * 0.5;
  float subpixF = subpixD * subpixE;
  bool lumaMLTZero = lumaMM < 0.0;
  /*--------------------------------------------------------------------------*/
  lumaEndN -= lumaNN * 0.5;
  lumaEndP -= lumaNN * 0.5;
  bool doneN = abs(lumaEndN) >= gradientScaled;
  bool doneP = abs(lumaEndP) >= gradientScaled;
  if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;
  if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;
  bool doneNP = (!doneN) || (!doneP);
  if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;
  if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;
  /*--------------------------------------------------------------------------*/
  if(doneNP) {
    if(!doneN) lumaEndN = luma(texTop(colorImg, posN.xy));
    if(!doneP) lumaEndP = luma(texTop(colorImg, posP.xy));
    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
    doneN = abs(lumaEndN) >= gradientScaled;
    doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;
    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;
    doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;
    if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;
  }
  /*--------------------------------------------------------------------------*/
  float dstN = posM.x - posN.x;
  float dstP = posP.x - posM.x;
  if(!horzSpan) dstN = posM.y - posN.y;
  if(!horzSpan) dstP = posP.y - posM.y;
  /*--------------------------------------------------------------------------*/
  bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
  float spanLength = (dstP + dstN);
  bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
  float spanLengthRcp = 1.0 / spanLength;
  /*--------------------------------------------------------------------------*/
  bool directionN = dstN < dstP;
  float dstMin = min(dstN, dstP);
  bool goodSpan = directionN ? goodSpanN : goodSpanP;
  float subpixG = subpixF * subpixF;
  float pixelOffset = (dstMin * (-spanLengthRcp)) + 0.5;
  float subpixH = subpixG * fxaaQualitySubpix;
  /*--------------------------------------------------------------------------*/
  float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
  float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
  if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
  if(horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
  fragColor = texTop(colorImg, posM);
}