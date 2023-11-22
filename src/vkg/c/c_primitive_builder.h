#ifndef VKG_C_PRIMITIVE_BUILDER_HPP
#define VKG_C_PRIMITIVE_BUILDER_HPP

#include <cstdint>
#include "c_vec.h"
#include "c_primitive.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CPrimitiveBuilder;
typedef struct CPrimitiveBuilder CPrimitiveBuilder;

CPrimitiveBuilder *NewPrimitiveBuilder();
void DeletePrimitiveBuilder(CPrimitiveBuilder *builder);

void BuildNewPrimitive(CPrimitiveBuilder *builder, CPrimitiveTopology topology);

void PrimitiveBuilderFrom(
    CPrimitiveBuilder *builder, cvec3 *positions, uint32_t position_offset_float, uint32_t numPositions, cvec3 *normals,
    uint32_t normal_offset_float, uint32_t numNormals, cvec2 *uvs, uint32_t uv_offset_float, uint32_t numUVs,
    uint32_t *indices, uint32_t numIndices);

void BuildTriangle(
    CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float, cvec3 *p3,
    uint32_t p3_offset_float);
void BuildRectangle(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float);
void BuildGrid(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy);
void BuildGridPatch(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy);
void BuildCheckerboard(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy);
void BuildCircle(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    int segments);
void BuildSphere(CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, float R, int nsubd);
void BuildBox(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float, float z);
void BuildBoxLine(
    CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float, cvec3 *up,
    uint32_t up_offset_float, float width, float height);
void BuildCone(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    int segments);
void BuildCylinder(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    bool cap, int segments);
void BuildAxis(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, float length, float R, float capLength,
    int segments);
void BuildLine(CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float);
void BuildRectangleLine(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float);

uint32_t BuilderNumPrimitives(CPrimitiveBuilder *builder);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_PRIMITIVE_BUILDER_HPP
