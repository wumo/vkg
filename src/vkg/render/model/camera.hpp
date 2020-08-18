#pragma once
#include "vkg/math/glm_common.hpp"
#include "vkg/math/matrices.hpp"

namespace vkg {
class Camera {
public:
  // ref in shaders
  struct Desc {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 projView;
    glm::vec4 eye;
    glm::vec4 r, v;
    float w, h, fov;
    float zNear, zFar;
    uint32_t frame;
  };

  Camera(
    const glm::vec3 &location, const glm::vec3 &focus, const glm::vec3 &worldUp,
    float fov, float zNear, float zFar, uint32_t width, uint32_t height);

  auto location() const -> glm::vec3;
  auto setLocation(glm::vec3 location) -> void;
  auto direction() const -> glm::vec3;
  auto setDirection(glm::vec3 direction) -> void;
  auto worldUp() const -> glm::vec3;
  auto setWorldUp(glm::vec3 worldUp) -> void;
  auto resize(uint32_t width, uint32_t height) -> void;
  auto zFar() const -> float;
  auto zNear() const -> float;
  auto fov() -> float;
  auto setZFar(float zFar) -> void;
  auto setZNear(float zNear) -> void;
  auto view() const -> glm::mat4;
  auto proj() const -> glm::mat4;
  auto width() const -> uint32_t;
  auto height() const -> uint32_t;
  auto desc() -> Desc;

protected:
  glm::vec3 location_;
  glm::vec3 focus_;
  glm::vec3 worldUp_;
  float fov_;
  float zNear_, zFar_;
  uint32_t width_, height_;
};
}