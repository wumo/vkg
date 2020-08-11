#include "panning_camera.hpp"

namespace vkg {
PanningCamera::PanningCamera(Camera &camera): camera(camera) {}

auto PanningCamera::xzIntersection(float inputX, float inputY) const -> glm::vec3 {
  auto view = camera.view();
  auto viewInv = glm::inverse(view);
  auto proj = camera.proj();
  auto c =
    (glm::vec2(inputX, inputY) + 0.5f) / glm::vec2(camera.width(), camera.height());
  c = c * 2.f - 1.f;
  auto projInv = glm::inverse(proj);
  auto target = glm::vec3(projInv * glm::vec4(c.x, c.y, 0, 1));
  auto dir = glm::vec3(viewInv * glm::vec4(normalize(target), 0));
  if(dir.y == 0) return camera.location() + camera.direction();
  auto t = -camera.location().y / dir.y;
  auto p = camera.location() + t * dir;
  return p;
}

auto PanningCamera::update(Input &input) -> void {
  static auto mouseRightPressed{false};
  auto front = camera.direction();
  auto len = length(front);
  camera.setLocation(
    camera.location() +
    normalize(front) * len * glm::clamp(float(input.scrollYOffset) / 10.f, -1.f, 1.f));
  input.scrollYOffset = 0;

  auto right = normalize(cross(front, camera.worldUp()));

  if(input.mouseButtonPressed[MouseButton::MouseButtonLeft]) {
    if(!rotate.start) {
      rotate.lastX = input.mousePosX;
      rotate.lastY = input.mousePosY;
      rotate.start = true;
    }
    auto xoffset = float(input.mousePosX - rotate.lastX);
    auto yoffset = float(input.mousePosY - rotate.lastY);
    rotate.lastX = input.mousePosX;
    rotate.lastY = input.mousePosY;

    xoffset *= -rotate.sensitivity;
    yoffset *= -rotate.sensitivity;

    auto focus = camera.location() + camera.direction();
    auto translation = -front;
    auto pitch = glm::degrees(angle(camera.worldUp(), normalize(translation)));
    pitch += yoffset;
    if(1 >= pitch || pitch >= 179) yoffset = 0;
    camera.setLocation(
      focus + angleAxis(glm::radians(xoffset), camera.worldUp()) *
                angleAxis(glm::radians(yoffset), right) * translation);
  } else
    rotate.start = false;

  if(input.mouseButtonPressed[MouseButton::MouseButtonRight]) {
    mouseRightPressed = true;
    if(!panning.start) {
      panning.lastX = input.mousePosX;
      panning.lastY = input.mousePosY;
      panning.start = true;
    }
    auto p0 = xzIntersection(panning.lastX, panning.lastY);
    auto p1 = xzIntersection(input.mousePosX, input.mousePosY);

    panning.lastX = input.mousePosX;
    panning.lastY = input.mousePosY;

    auto translation = p0 - p1;
    camera.setDirection(camera.direction() + translation);
    camera.setLocation(camera.location() + translation);
  } else {
    if(mouseRightPressed) { panning.start = false; }
    mouseRightPressed = false;
  }
}
}