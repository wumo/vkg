#include "light.hpp"
#include "vkg/render/scene.hpp"

namespace vkg {
Lighting::Lighting(Scene &scene): desc{scene.allocateLightingDesc()} {}
auto Lighting::numLights() const -> uint32_t { return numLights_; }
auto Lighting::exposure() const -> float { return exposure_; }
auto Lighting::gamma() const -> float { return gamma_; }
auto Lighting::setNumLights(uint32_t numLights) -> void {
  numLights_ = numLights;
  desc.ptr->numLights = numLights;
}
auto Lighting::setExposure(float exposure) -> void {
  exposure_ = exposure;
  desc.ptr->exposure = exposure;
}
auto Lighting::setGamma(float gamma) -> void {
  gamma_ = gamma;
  desc.ptr->gamma = gamma;
}
Light::Light(Scene &scene, uint32_t id, uint32_t count)
  : scene{scene}, id_{id}, count_{count} {
  descs.resize(count);
  for(int i = 0; i < count; ++i) {
    descs[i] = scene.allocateLightDesc();
    *descs[i].ptr = {color_, intensity_, location_, range_};
  }
}
auto Light::id() const -> uint32_t { return id_; }
auto Light::count() const -> uint32_t { return count_; }
auto Light::color() const -> glm::vec3 { return color_; }
auto Light::intensity() const -> float { return intensity_; }
auto Light::location() const -> glm::vec3 { return location_; }
auto Light::range() const -> float { return range_; }
auto Light::setColor(glm::vec3 color) -> void {
  color_ = color;
  scene.scheduleFrameUpdate(Update::Type::Light, id_, count_, ticket);
}
auto Light::setIntensity(float intensity) -> void {
  intensity_ = intensity;
  scene.scheduleFrameUpdate(Update::Type::Light, id_, count_, ticket);
}
auto Light::setLocation(glm::vec3 location) -> void {
  location_ = location;
  scene.scheduleFrameUpdate(Update::Type::Light, id_, count_, ticket);
}
auto Light::setRange(float range) -> void {
  range_ = range;
  scene.scheduleFrameUpdate(Update::Type::Light, id_, count_, ticket);
}
void Light::updateDesc(uint32_t frameIdx) {
  *descs[std::clamp(frameIdx, 0u, count_ - 1)].ptr = {
    color_, intensity_, location_, range_};
}
}