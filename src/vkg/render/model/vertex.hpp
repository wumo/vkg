#pragma once
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct Vertex {
  using Position = glm::vec3;
  using Normal = glm::vec3;
  using Tangent = glm::vec3;
  using UV = glm::vec2;
  using Color = glm::vec3;
  using Joint = glm::vec4;
  using Weight = glm::vec4;
};
}
