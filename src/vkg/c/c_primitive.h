#ifndef VKG_C_PRIMITIVE_HPP
#define VKG_C_PRIMITIVE_HPP
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#else
  #include <stdbool.h>
#endif

enum CPrimitiveTopology {
  CPrimitiveTriangles = 1u,
  CPrimitiveLines = 2u,
  CPrimitiveProcedural = 3u,
  CPrimitivePatches = 4u
};

struct CScene;
struct CPrimitiveBuilder;

uint32_t PrimitiveGetCount(CScene *scene, uint32_t id);
CPrimitiveTopology PrimitiveGetTopology(CScene *scene, uint32_t id);
CUintRange PrimitiveGetIndex(CScene *scene, uint32_t id, uint32_t idx);
CUintRange PrimitiveGetPosition(CScene *scene, uint32_t id, uint32_t idx);
CUintRange PrimitiveGetNormal(CScene *scene, uint32_t id, uint32_t idx);
CUintRange PrimitiveGetUV(CScene *scene, uint32_t id, uint32_t idx);
caabb PrimitiveGetAABB(CScene *scene, uint32_t id, uint32_t idx);
void PrimitiveSetAABB(CScene *scene, uint32_t id, caabb *aabb, uint32_t idx);

void PrimitiveUpdate(
  CScene *scene, uint32_t id, uint32_t idx, cvec3 *positions,
  uint32_t position_offset_float, uint32_t numPositions, cvec3 *normals,
  uint32_t normal_offset_float, uint32_t numNormals, caabb *aabb);

void PrimitiveUpdateFromBuilder(
  CScene *scene, uint32_t id, uint32_t idx, CPrimitiveBuilder *builder);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_PRIMITIVE_HPP
