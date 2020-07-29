#pragma once
#include <cstdint>
#include <vector>
#include "vkg/math/glm_common.hpp"

namespace vkg {
enum class PathType { Translation, Rotation, Scale };
class AnimationChannel {
public:
  PathType path;
  uint32_t node;
  uint32_t samplerIdx;
  float prevTime{0};
  uint32_t prevKey{0};
};
enum class InterpolationType { Linear, Step, CubicSpline };
class AnimationSampler {
public:
  InterpolationType interpolation;
  std::vector<float> keyTimings;
  std::vector<glm::vec4> keyFrames;

  glm::vec4 cubicSpline(uint32_t key, uint32_t nextKey, float keyDelta, float t) const;
  glm::vec4 interpolateRotation(uint32_t key, uint32_t nextKey, float t) const;
  glm::vec4 linear(uint32_t key, uint32_t nextKey, float t) const;
};

class Scene;

class Animation {
public:
  explicit Animation(Scene &scene);
  auto reset(uint32_t index) -> void;
  auto resetAll() -> void;
  auto animate(uint32_t index, float elapsedMs) -> void;
  auto animateAll(float elapsedMs) -> void;

  std::string name;
  std::vector<AnimationSampler> samplers;
  std::vector<AnimationChannel> channels;

private:
  Scene &scene;
};
}