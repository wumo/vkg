#ifndef VKG_C_MODEL_H
#define VKG_C_MODEL_H

#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CScene;

struct CAnimation;
typedef struct CAnimation CAnimation;

uint32_t ModelNumNodes(CScene *scene, uint32_t id);
void ModelGetNodes(CScene *scene, uint32_t id, uint32_t *nodes);
void ModelGetAABB(CScene *scene, uint32_t id, caabb *aabb);
uint32_t ModelNumAnimations(CScene *scene, uint32_t id);
CAnimation *ModelGetAnimation(CScene *scene, uint32_t id, uint32_t idx);

void AnimationReset(CAnimation *animation, uint32_t idx);
void AnimationResetAll(CAnimation *animation);
void AnimationAnimate(CAnimation *animation, uint32_t idx, float elapsedMs);
void AnimationAnimateAll(CAnimation *animation, float elapsedMs);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_MODEL_H
