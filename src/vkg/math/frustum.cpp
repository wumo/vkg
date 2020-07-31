#include "frustum.hpp"

namespace vkg {
Frustum::Frustum(glm::mat4 m) { update(m); }
auto Frustum::update(glm::mat4 m) -> void {
  planes[0] = glm::row(m, 3) + glm::row(m, 0); //left clip plane
  planes[1] = glm::row(m, 3) - glm::row(m, 0); //right clip plane
  planes[2] = glm::row(m, 3) + glm::row(m, 1); //bottom clip plane
  planes[3] = glm::row(m, 3) - glm::row(m, 1); //top clip plane
  planes[4] = glm::row(m, 2);                  //near clip plane
  planes[5] = glm::row(m, 3) - glm::row(m, 2); //far clip plane

  for(auto &plane: planes) {
    float len = glm::length(glm::vec3(plane));
    if(glm::epsilonEqual(len, 0.f, std::numeric_limits<float>::epsilon())) continue;
    plane /= len;
  }
}
auto Frustum::contains(glm::vec3 p) -> bool {
  auto _p = glm::vec4(p, 1);
  for(const auto &plane: planes)
    if(glm::dot(plane, _p) < 0) return false;
  return true;
}
}