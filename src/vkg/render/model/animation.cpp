#include "animation.hpp"
#include "vkg/render/scene.hpp"
#include <algorithm>

namespace vkg {

auto AnimationSampler::cubicSpline(
  uint32_t key, uint32_t nextKey, float keyDelta, float t) const -> glm::vec4 {
  auto prevIdx = key * 3;
  auto nextIdx = nextKey * 3;

  float tSq = std::pow(t, 2);
  float tCub = std::pow(t, 3);

  auto v0 = keyFrames[prevIdx + 1];
  auto a = keyDelta * keyFrames[nextIdx];
  auto b = keyDelta * keyFrames[prevIdx + 2];
  auto v1 = keyFrames[nextIdx + 1];

  return (2 * tCub - 3 * tSq + 1) * v0 + (tCub - 2 * tSq + t) * b +
         (-2 * tCub + 3 * tSq) * v1 + (tCub - tSq) * a;
}

auto AnimationSampler::interpolateRotation(uint32_t key, uint32_t nextKey, float tn) const
  -> glm::vec4 {
  auto _q0 = keyFrames[key];
  auto _q1 = keyFrames[nextKey];
  glm::quat q0{_q0.w, _q0.x, _q0.y, _q0.z};
  glm::quat q1{_q1.w, _q1.x, _q1.y, _q1.z};
  auto result = glm::slerp(q0, q1, tn);
  return {result.x, result.y, result.z, result.w};
}

auto AnimationSampler::linear(uint32_t key, uint32_t nextKey, float tn) const
  -> glm::vec4 {
  return glm::mix(keyFrames[key], keyFrames[nextKey], tn);
}

Animation::Animation(Scene &scene): scene{scene} {}

auto Animation::reset(uint32_t index) -> void {
  channels[index].prevTime = 0;
  channels[index].prevKey = 0;
}

auto Animation::resetAll() -> void {
  for(auto i = 0u; i < channels.size(); ++i)
    reset(i);
}

auto Animation::animate(uint32_t index, float elapsedMs) -> void {
  auto &channel = channels[index];
  auto &sampler = samplers[channel.samplerIdx];

  auto &node = scene.node(channel.node);
  auto transform = node.transform();
  glm::vec4 result{};
  if(sampler.keyTimings.size() == 1) result = sampler.keyFrames[0];
  else {
    auto t = channel.prevTime;
    t += elapsedMs / 1000;
    t = std::max(std::fmod(t, sampler.keyTimings.back()), sampler.keyTimings.front());
    if(channel.prevTime > t) channel.prevKey = 0;
    channel.prevTime = t;
    auto nextKey = 0;
    for(int i = 0; i < sampler.keyTimings.size(); ++i)
      if(t <= sampler.keyTimings[i]) {
        nextKey = std::clamp(i, 1, int(sampler.keyTimings.size() - 1));
        break;
      }
    channel.prevKey = std::clamp(nextKey - 1, 0, nextKey);
    auto keyDelta = sampler.keyTimings[nextKey] - sampler.keyTimings[channel.prevKey];
    auto tn = (t - sampler.keyTimings[channel.prevKey]) / keyDelta;
    if(channel.path == PathType::Rotation) {
      if(sampler.interpolation == InterpolationType::CubicSpline)
        result = sampler.cubicSpline(channel.prevKey, nextKey, keyDelta, tn);
      else
        result = sampler.interpolateRotation(channel.prevKey, nextKey, tn);
    } else {
      switch(sampler.interpolation) {
        case InterpolationType::Linear:
          result = sampler.linear(channel.prevKey, nextKey, tn);
          break;
        case InterpolationType::Step: result = sampler.keyFrames[channel.prevKey]; break;
        case InterpolationType::CubicSpline:
          result = sampler.cubicSpline(channel.prevKey, nextKey, keyDelta, tn);
          break;
      }
    }
  }
  switch(channel.path) {
    case PathType::Translation: transform.translation = result; break;
    case PathType::Rotation:
      transform.rotation =
        glm::normalize(glm::quat{result.w, result.x, result.y, result.z});
      break;
    case PathType::Scale: transform.scale = result; break;
  }
  node.setTransform(transform);
}

void Animation::animateAll(float elapsedMs) {
  for(auto i = 0u; i < channels.size(); ++i)
    animate(i, elapsedMs);
}

}