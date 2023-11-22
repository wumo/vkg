#ifndef VKG_C_MATERIAL_H
#define VKG_C_MATERIAL_H
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

enum CMaterialType {
    /**BRDF without reflection trace*/
    CMaterialBRDF = 0x1u,
    CMaterialBRDFSG = 0x2u,
    /**BRDF with reflection trace*/
    CMaterialReflective = 0x4u,
    /**BRDF with reflection trace and refraction trace*/
    CMaterialRefractive = 0x8u,
    /**diffuse coloring*/
    CMaterialNone = 0x10u,
    /**transparent color blending*/
    CMaterialTransparent = 0x20u,
    /**terrain */
    CMaterialTerrain = 0x40u
};

struct CScene;

uint32_t MaterialGetCount(CScene *scene, uint32_t id);
CMaterialType MaterialGetType(CScene *scene, uint32_t id);
uint32_t MaterialGetColorTex(CScene *scene, uint32_t id);
void MaterialSetColorTex(CScene *scene, uint32_t id, uint32_t colorTex);
uint32_t MaterialGetPbrTex(CScene *scene, uint32_t id);
void MaterialSetPbrTex(CScene *scene, uint32_t id, uint32_t pbrTex);
uint32_t MaterialGetNormalTex(CScene *scene, uint32_t id);
void MaterialSetNormalTex(CScene *scene, uint32_t id, uint32_t normalTex);
uint32_t MaterialGetOcclusionTex(CScene *scene, uint32_t id);
void MaterialSetOcclusionTex(CScene *scene, uint32_t id, uint32_t occlusionTex);
uint32_t MaterialGetEmissiveTex(CScene *scene, uint32_t id);
void MaterialSetEmissiveTex(CScene *scene, uint32_t id, uint32_t emissiveTex);
void MaterialGetColorFactor(CScene *scene, uint32_t id, cvec4 *colorFactor, uint32_t offset_float);
void MaterialSetColorFactor(CScene *scene, uint32_t id, cvec4 *colorFactor, uint32_t offset_float);
void MaterialGetPbrFactor(CScene *scene, uint32_t id, cvec4 *pbrFactor, uint32_t offset_float);
void MaterialSetPbrFactor(CScene *scene, uint32_t id, cvec4 *pbrFactor, uint32_t offset_float);
float MaterialGetOcclusionStrength(CScene *scene, uint32_t id);
void MaterialSetOcclusionStrength(CScene *scene, uint32_t id, float occlusionStrength);
float MaterialGetAlphaCutoff(CScene *scene, uint32_t id);
void MaterialSetAlphaCutoff(CScene *scene, uint32_t id, float alphaCutoff);
void MaterialGetEmissiveFactor(CScene *scene, uint32_t id, cvec4 *emissiveFactor, uint32_t offset_float);
void MaterialSetEmissiveFactor(CScene *scene, uint32_t id, cvec4 *emissiveFactor, uint32_t offset_float);
uint32_t MaterialGetHeightTex(CScene *scene, uint32_t id);
void MaterialSetHeightTex(CScene *scene, uint32_t id, uint32_t heightTex);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_MATERIAL_H
