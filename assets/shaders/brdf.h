#ifndef VKG_BRDF_H
#define VKG_BRDF_H

//
// This fragment shader defines a reference implementation for Physically Based Shading of
// a microfacet surface material defined by a glTF model.
// See https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/pbr.frag
//
// References:
// [1] Real Shading in Unreal Engine 4
//     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// [2] Physically Based Shading at Disney
//     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// [3] README.md - Environment Maps
//     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
// [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
//     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
struct MaterialInfo {
  // roughness value, as authored by the model creator (input to shader)
  float perceptualRoughness;
  // roughness mapped to a more linear change in the roughness (proposed by [2])
  float alphaRoughness;
  vec3 diffuseColor;  // color contribution from diffuse lighting
  vec3 specularColor; // color contribution from specular lighting
  vec3 reflectance0;  // full reflectance color (normal incidence angle)
  vec3 reflectance90; // reflectance color at grazing angle
  float ao;
  vec3 emissive;
  float eta; //refractive eta
};

struct AngularInfo {
  float NdotL; // cos angle between normal and light direction
  float NdotV; // cos angle between normal and view direction
  float NdotH; // cos angle between normal and half vector
  float LdotH; // cos angle between light direction and half vector
  float VdotH; // cos angle between view direction and half vector
};

float getPerceivedBrightness(vec3 vector) {
  return sqrt(
    0.299 * vector.r * vector.r + 0.587 * vector.g * vector.g +
    0.114 * vector.b * vector.b);
}

const float c_MinReflectance = 0.04;
// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness/examples/convert-between-workflows/js/three.pbrUtilities.js#L34
float solveMetallic(vec3 diffuse, vec3 specular, float oneMinusSpecularStrength) {
  float specularBrightness = getPerceivedBrightness(specular);

  if(specularBrightness < c_MinReflectance) { return 0.0; }

  float diffuseBrightness = getPerceivedBrightness(diffuse);

  float a = c_MinReflectance;
  float b = diffuseBrightness * oneMinusSpecularStrength / (1.0 - c_MinReflectance) +
            specularBrightness - 2.0 * c_MinReflectance;
  float c = c_MinReflectance - specularBrightness;
  float D = b * b - 4.0 * a * c;

  return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
// F_Schlick
vec3 F_Schlick(float u, vec3 f0, vec3 f90) {
  return f0 + (f90 - f0) * pow(clamp(1.0 - u, 0.0, 1.0), 5.0);
}

#define diffuse Fr_DisneyDiffuse

// Lambert lighting
// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
vec3 Diffuse_Lambert(MaterialInfo materialInfo, AngularInfo angularInfo) {
  return materialInfo.diffuseColor / M_PI;
}

vec3 Diffuse_Burley(MaterialInfo materialInfo, AngularInfo angularInfo) {
  float f90 =
    0.5 + 2 * materialInfo.alphaRoughness * angularInfo.LdotH * angularInfo.LdotH;
  vec3 lightScatter = F_Schlick(angularInfo.NdotL, vec3(1.0), vec3(f90));
  vec3 viewScatter = F_Schlick(angularInfo.NdotV, vec3(1.0), vec3(f90));
  return materialInfo.diffuseColor * lightScatter * viewScatter * (1.0 / M_PI);
}

// Renormalized Disney Diffuse
// see https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
vec3 Fr_DisneyDiffuse(MaterialInfo materialInfo, AngularInfo angularInfo) {
  float energyBias = mix(0, 0.5, materialInfo.alphaRoughness);
  float energyFactor = mix(1, 1 / 1.51, materialInfo.alphaRoughness);
  float f90 =
    energyBias + 2 * materialInfo.alphaRoughness * angularInfo.LdotH * angularInfo.LdotH;
  vec3 lightScatter = F_Schlick(angularInfo.NdotL, vec3(1.0), vec3(f90));
  vec3 viewScatter = F_Schlick(angularInfo.NdotV, vec3(1.0), vec3(f90));
  return materialInfo.diffuseColor * lightScatter * viewScatter * (1.0 / M_PI);
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
// F_Schlick
vec3 specularReflection(MaterialInfo materialInfo, AngularInfo angularInfo) {
  return materialInfo.reflectance0 +
         (materialInfo.reflectance90 - materialInfo.reflectance0) *
           pow(clamp(1.0 - angularInfo.VdotH, 0.0, 1.0), 5.0);
}

// Smith Joint GGX
// V_SmithGGXCorrelated
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float visibilityOcclusion(MaterialInfo materialInfo, AngularInfo angularInfo) {
  float NdotL = angularInfo.NdotL;
  float NdotV = angularInfo.NdotV;
  float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness;

  float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
  float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

  float GGX = GGXV + GGXL;
  if(GGX > 0.0) { return 0.5 / GGX; }
  return 0.0;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
// D_GGX
float microfacetDistribution(MaterialInfo materialInfo, AngularInfo angularInfo) {
  float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness;
  float f =
    (angularInfo.NdotH * alphaRoughnessSq - angularInfo.NdotH) * angularInfo.NdotH + 1.0;
  return alphaRoughnessSq / (M_PI * f * f);
}

AngularInfo getAngularInfo(vec3 pointToLight, vec3 normal, vec3 view) {
  // Standard one-letter names
  vec3 n = normalize(normal);       // Outward direction of surface point
  vec3 v = normalize(view);         // Direction from surface point to view
  vec3 l = normalize(pointToLight); // Direction from surface point to light
  vec3 h = normalize(l + v);        // Direction of the vector between l and v

  float NdotL = clamp(dot(n, l), 0.05, 1.0);
  float NdotV = clamp(dot(n, v), 0.0, 1.0);
  float NdotH = clamp(dot(n, h), 0.0, 1.0);
  float LdotH = clamp(dot(l, h), 0.0, 1.0);
  float VdotH = clamp(dot(v, h), 0.0, 1.0);

  return AngularInfo(NdotL, NdotV, NdotH, LdotH, VdotH);
}

vec3 brdf(vec3 rayDir, MaterialInfo materialInfo, vec3 normal, vec3 view, out vec3 F) {
  AngularInfo angularInfo = getAngularInfo(rayDir, normal, view);
  // If one of the dot products is larger than zero, no division by zero can happen. Avoids black borders.
  if(angularInfo.NdotL > 0.0 || angularInfo.NdotV > 0.0) {
    // Calculate the shading terms for the microfacet specular shading model
    F = specularReflection(materialInfo, angularInfo);
    float Vis = visibilityOcclusion(materialInfo, angularInfo);
    float D = microfacetDistribution(materialInfo, angularInfo);

    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * diffuse(materialInfo, angularInfo);
    vec3 specContrib = F * Vis * D;

    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    return angularInfo.NdotL * (diffuseContrib + specContrib);
  }
  return vec3(0.0, 0.0, 0.0);
}

vec3 fresnelReflectance(MaterialInfo materialInfo, vec3 rayDir, vec3 normal, vec3 view) {
  AngularInfo angularInfo = getAngularInfo(rayDir, normal, view);
  return specularReflection(materialInfo, angularInfo);
}

// Utility function to get a vector perpendicular to an input vector
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
vec3 getPerpendicularStark(vec3 u) {
  vec3 a = abs(u);
  uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
  uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
  uint zm = 1 ^ (xm | ym);
  return cross(u, vec3(xm, ym, zm));
}

// Get a GGX half vector / microfacet normal, sampled according to the GGX distribution
//    When using this function to sample, the probability density is pdf = D * NdotH / (4 * HdotV)
//
//    \param[in] u Uniformly distributed random numbers between 0 and 1
//    \param[in] N Surface normal
//    \param[in] roughness Roughness^2 of material
//
vec3 getGGXMicrofacet(vec2 u, vec3 N, float roughness) {
  float a2 = roughness * roughness;

  float phi = 2 * M_PI * u.x;
  float cosTheta = sqrt(max(0, (1 - u.y)) / (1 + (a2 - 1) * u.y));
  float sinTheta = sqrt(max(0, 1 - cosTheta * cosTheta));

  // Tangent space H
  vec3 tH;
  tH.x = sinTheta * cos(phi);
  tH.y = sinTheta * sin(phi);
  tH.z = cosTheta;

  vec3 T = getPerpendicularStark(N);
  vec3 B = normalize(cross(N, T));

  // World space H
  return normalize(T * tH.x + B * tH.y + N * tH.z);
}

#endif //VKG_RT_LIGHTING_H