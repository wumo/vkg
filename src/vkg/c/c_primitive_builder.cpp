#include "c_primitive_builder.h"
#include "vkg/render/builder/primitive_builder.hpp"

using namespace vkg;

CPrimitiveBuilder *NewPrimitiveBuilder() { return reinterpret_cast<CPrimitiveBuilder *>(new PrimitiveBuilder()); }
void DeletePrimitiveBuilder(CPrimitiveBuilder *builder) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    delete builder_;
}
void BuildNewPrimitive(CPrimitiveBuilder *builder, CPrimitiveTopology topology) {
    auto *_builder = reinterpret_cast<PrimitiveBuilder *>(builder);
    auto _topology = static_cast<PrimitiveTopology>(topology);
    _builder->newPrimitive(_topology);
}
void PrimitiveBuilderFrom(
    CPrimitiveBuilder *builder, cvec3 *positions, uint32_t position_offset_float, uint32_t numPositions, cvec3 *normals,
    uint32_t normal_offset_float, uint32_t numNormals, cvec2 *uvs, uint32_t uv_offset_float, uint32_t numUVs,
    uint32_t *indices, uint32_t numIndices) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->from(
        {(Vertex::Position *)((float *)positions + position_offset_float), numPositions},
        {(Vertex::Normal *)((float *)normals + normal_offset_float), numNormals},
        {(Vertex::UV *)((float *)uvs + uv_offset_float), numUVs}, {indices, numIndices});
}
void BuildTriangle(
    CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float, cvec3 *p3,
    uint32_t p3_offset_float) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->triangle(
        *(glm::vec3 *)((float *)p1 + p1_offset_float), *(glm::vec3 *)((float *)p2 + p2_offset_float),
        *(glm::vec3 *)((float *)p3 + p3_offset_float));
}
void BuildRectangle(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->rectangle(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float));
}
void BuildGrid(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->grid(
        nx, ny, *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float), wx, wy);
}
void BuildGridPatch(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->gridPatch(
        nx, ny, *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float), wx, wy);
}
void BuildCheckerboard(
    CPrimitiveBuilder *builder, uint32_t nx, uint32_t ny, cvec3 *center, uint32_t center_offset_float, cvec3 *x,
    uint32_t x_offset_float, cvec3 *y, uint32_t y_offset_float, float wx, float wy) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->checkerboard(
        nx, ny, *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float), wx, wy);
}
void BuildCircle(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    int segments) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->circle(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)z + z_offset_float), R,
        segments);
}
void BuildSphere(CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, float R, int nsubd) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->sphere(*(glm::vec3 *)((float *)center + center_offset_float), R, nsubd);
}
void BuildBox(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float, float z) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->box(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float), z);
}
void BuildBoxLine(
    CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float, cvec3 *up,
    uint32_t up_offset_float, float width, float height) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->box(
        *(glm::vec3 *)((float *)p1 + p1_offset_float), *(glm::vec3 *)((float *)p2 + p2_offset_float),
        *(glm::vec3 *)((float *)up + up_offset_float), width, height);
}
void BuildCone(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    int segments) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->cone(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)z + z_offset_float), R,
        segments);
}
void BuildCylinder(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *z, uint32_t z_offset_float, float R,
    bool cap, int segments) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->cylinder(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)z + z_offset_float), R, cap,
        segments);
}
void BuildAxis(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, float length, float R, float capLength,
    int segments) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->axis(*(glm::vec3 *)((float *)center + center_offset_float), length, R, capLength, segments);
}
void BuildLine(CPrimitiveBuilder *builder, cvec3 *p1, uint32_t p1_offset_float, cvec3 *p2, uint32_t p2_offset_float) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->line(*(glm::vec3 *)((float *)p1 + p1_offset_float), *(glm::vec3 *)((float *)p2 + p2_offset_float));
}
void BuildRectangleLine(
    CPrimitiveBuilder *builder, cvec3 *center, uint32_t center_offset_float, cvec3 *x, uint32_t x_offset_float,
    cvec3 *y, uint32_t y_offset_float) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    builder_->rectangleLine(
        *(glm::vec3 *)((float *)center + center_offset_float), *(glm::vec3 *)((float *)x + x_offset_float),
        *(glm::vec3 *)((float *)y + y_offset_float));
}
uint32_t BuilderNumPrimitives(CPrimitiveBuilder *builder) {
    auto *builder_ = reinterpret_cast<PrimitiveBuilder *>(builder);
    return builder_->primitives().size();
}