#include "camera.hpp"

namespace vkg {

Camera::Camera(
  Allocation<Desc> desc, const glm::vec3 &location, const glm::vec3 &focus,
  const glm::vec3 &worldUp, float fov, float zNear, float zFar, uint32_t width,
  uint32_t height)
  : desc{desc},
    location_(location),
    focus_(focus),
    worldUp_(worldUp),
    fov_(fov),
    zNear_(zNear),
    zFar_(zFar),
    width_(width),
    height_(height) {}
auto Camera::location() const -> glm::vec3 { return location_; }
auto Camera::direction() const -> glm::vec3 { return focus_ - location_; }
auto Camera::worldUp() const -> glm::vec3 { return worldUp_; }
auto Camera::view() const -> glm::mat4 { return viewMatrix(location_, focus_, worldUp_); }
auto Camera::proj() const -> glm::mat4 {
  return perspectiveMatrix(fov_, float(width_) / float(height_), zNear_, zFar_);
}
auto Camera::width() const -> uint32_t { return width_; }
auto Camera::height() const -> uint32_t { return height_; }
auto Camera::zFar() -> float { return zFar_; }
auto Camera::zNear() -> float { return zNear_; }

auto Camera::setLocation(glm::vec3 location) -> void { location_ = location; }
auto Camera::setDirection(glm::vec3 direction) -> void { focus_ = location_ + direction; }
auto Camera::setWorldUp(glm::vec3 worldUp) -> void { worldUp_ = worldUp; }
auto Camera::resize(uint32_t width, uint32_t height) -> void {
  width_ = width;
  height_ = height;
}
auto Camera::setZFar(float zFar) -> void { zFar_ = zFar; }
auto Camera::setZNear(float zNear) -> void { zNear_ = zNear; }

auto Camera::updateUBO() -> void {
  auto proj_ = proj();
  auto view_ = view();
  auto projView = proj_ * view_;
  auto v = glm::vec4(normalize(focus_ - location_), 1);
  auto r = glm::vec4(normalize(cross(glm::vec3(v), worldUp_)), 1);
  *desc.ptr = {view_, proj_,  projView,      glm::vec4(location_, 1.0),
               r,     v,      float(width_), float(height_),
               fov_,  zNear_, zFar_};
}
auto Camera::fov() -> float { return fov_; }

}