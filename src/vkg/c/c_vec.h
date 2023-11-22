#ifndef VKG_C_VEC_HPP
#define VKG_C_VEC_HPP

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

const static uint32_t CNullIdx = ~0U;

typedef struct {
    uint32_t start;
    uint32_t size;
} CUintRange;
typedef struct {
    float x, y;
} cvec2;
typedef struct {
    float x, y, z;
} cvec3;
typedef struct {
    float x, y, z, w;
} cvec4;
typedef struct {
    cvec3 c1, c2, c3;
} cmat3;
typedef struct {
    cvec4 c1, c2, c3, c4;
} cmat4;

typedef struct {
    cvec3 translation;
    cvec3 scale;
    cvec4 rotation;
} ctransform;

typedef struct {
    cvec3 min;
    cvec3 max;
} caabb;

void AABBMergePoint(caabb *aabb, cvec3 *p, uint32_t offset);
void AABBMergeAABB(caabb *aabb, caabb *other);
void AABBTransformMatrix(caabb *aabb, cmat4 *matrix, uint32_t offset);
void AABBTransform(caabb *aabb, ctransform *transform);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_VEC_HPP
