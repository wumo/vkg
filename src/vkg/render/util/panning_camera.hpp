#pragma once
#include "vkg/render/model/camera.hpp"
#include "vkg/base/window/input.h"

namespace vkg {
class PanningCamera {
public:
  explicit PanningCamera(Camera &camera);

  auto update(Input &input) -> void;

private:
  auto xzIntersection(float inputX, float inputY) const -> glm::vec3;

  Camera &camera;

  struct MouseAction {
    float lastX{0}, lastY{0};
    bool start{false};
    float sensitivity{0.1f};
  };

  MouseAction rotate, panning;
};
}
