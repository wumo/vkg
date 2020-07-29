#pragma once
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/model/primitive.hpp"
#include <par_shapes.h>
#include <span>

namespace vkg {
class PrimitiveBuilder {
public:
  struct PrimitiveUBO {
    UIntRange index, position, normal, uv;
    AABB aabb;
    PrimitiveTopology topology{PrimitiveTopology::Triangles};
    DynamicType type{DynamicType::Static};
  };

public:
  auto newPrimitive(
    PrimitiveTopology topology = PrimitiveTopology::Triangles,
    DynamicType type = DynamicType::Static) -> PrimitiveBuilder &;

  /**
   * construct the mesh from predefined vertices and indices
   * @param vertices
   * @param indices
   * @param primitive
   * @param checkIndexInRange check if index is out of the range of vertices
   */
  auto from(
    std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals,
    std::span<Vertex::UV> uvs, std::span<uint32_t> indices) -> PrimitiveBuilder &;

  /**generate a triangle from 3 nodes. Nodes should be in the Counter-Clock-Wise order.*/
  auto triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) -> PrimitiveBuilder &;
  /**
  * generate a rectangle
  * @param center the center of the rectangle
  * @param right the 'width' direction of the rectangle from the center.
  * @param up the 'height' direction of the rectangle from the center.
  */
  auto rectangle(glm::vec3 center, glm::vec3 right, glm::vec3 up) -> PrimitiveBuilder &;
  auto grid(
    uint32_t nx, uint32_t ny, glm::vec3 center = {0.f, 0.f, 0.f}, glm::vec3 x = {0, 0, 1},
    glm::vec3 y = {1, 0, 0}, float wx = 1, float wy = 1) -> PrimitiveBuilder &;
  auto gridPatch(
    uint32_t nx, uint32_t ny, glm::vec3 center = {0.f, 0.f, 0.f}, glm::vec3 x = {0, 0, 1},
    glm::vec3 y = {1, 0, 0}, float wx = 1, float wy = 1) -> PrimitiveBuilder &;
  auto checkerboard(
    uint32_t nx, uint32_t ny, glm::vec3 center = {0.f, 0.f, 0.f}, glm::vec3 x = {0, 0, 1},
    glm::vec3 y = {1, 0, 0}, float wx = 1, float wy = 1) -> PrimitiveBuilder &;

  /**
  * generate a circle
  * @param center the center of the circle
  * @param z the normal vector of the circle surface.
  * @param R the radius of the circle
  * @param segments number of the edges to simulate circle.
  */
  auto circle(glm::vec3 center, glm::vec3 z, float R, int segments = 50)
    -> PrimitiveBuilder &;
  /**
  * generate a sphere
  * @param center the center of the sphere
  * @param R  the radius of the sphere
  * @param nsubd the number of sub-divisions. At most 3, the higher,the smoother.
  */
  auto sphere(glm::vec3 center, float R, int nsubd = 3) -> PrimitiveBuilder &;
  /**
  * generate a box
  * @param center the center of the box
  * @param x the 'width' direction of the box from the center.
  * @param y the 'height' direction of the box from the center.
  * @param z the 'depth' direction of the box from the center.
  */
  auto box(glm::vec3 center, glm::vec3 x, glm::vec3 y, float z) -> PrimitiveBuilder &;
  auto box(glm::vec3 p1, glm::vec3 p2, glm::vec3 up, float width, float height)
    -> PrimitiveBuilder &;
  /**
  * generate a cone
  * @param center the center of the cone
  * @param z  the normal vector of the bottom circle surface of the cone.
  * @param R  the radius of the bottom circle surface of the cone.
  * @param segments  number of the edges of a polygon to simulate the circle.
  * @return
  */
  auto cone(glm::vec3 center, glm::vec3 z, float R, int segments = 50)
    -> PrimitiveBuilder &;
  /**
  * generate a cylinder
  * @param center the center of the cylinder
  * @param z the normal vector of the cylinder
  * @param R the radius of the bottom circle of the cylinder
  * @param cap the length between top and bottom circle of the cylinder.
  * @param segments  number of the edges of a polygon to simulate the circle.
  */
  auto cylinder(
    glm::vec3 center, glm::vec3 z, float R, bool cap = true, int segments = 50)
    -> PrimitiveBuilder &;
  /**
  * generate an axis model.
  * @param center the center of the center of the axis
  * @param length the length of each axis bar.
  * @param R  the radius of the cap of each axis bar.
  * @param capLength the length of the cap of each axis bar.
  * @param segments number of the edges of a polygon to simulate the circle.
  */
  auto axis(glm::vec3 center, float length, float R, float capLength, int segments = 10)
    -> PrimitiveBuilder &;
  /**
  * generate a line from p1 to p2.
  * @param p1
  * @param p2
  */
  auto line(glm::vec3 p1, glm::vec3 p2) -> PrimitiveBuilder &;
  /**
  * generate a line by a rectangle located in center and from x to y.
  * @param center the center of the line
  * @param x
  * @param y
  */
  auto rectangleLine(glm::vec3 center, glm::vec3 x, glm::vec3 y) -> PrimitiveBuilder &;

  auto positions() -> std::span<Vertex::Position>;
  auto normals() -> std::span<Vertex::Normal>;
  auto uvs() -> std::span<Vertex::UV>;
  auto indices() -> std::span<uint32_t>;
  auto primitives() -> std::span<PrimitiveUBO>;

private:
  auto currentVertexID() const -> uint32_t;

private:
  std::vector<Vertex::Position> _positions;
  std::vector<Vertex::Normal> _normals;
  std::vector<Vertex::UV> _uvs;
  std::vector<uint32_t> _indices;
  AABB aabb;
  std::vector<PrimitiveUBO> _primitives;
};
}
