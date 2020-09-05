#include "atmosphere.hpp"
namespace vkg {

auto AtmosphereSetting::isEnabled() const -> bool { return enabled_; }
auto AtmosphereSetting::enable(bool enabled) -> void { enabled_ = enabled; }
auto AtmosphereSetting::sunDirection() const -> const glm::vec3 & {
  return sunDirection_;
}
void AtmosphereSetting::setSunDirection(const glm::vec3 &sunDirection) {
  sunDirection_ = glm::normalize(sunDirection);
}
auto AtmosphereSetting::earthCenter() const -> const glm::vec3 & { return earthCenter_; }
void AtmosphereSetting::setEarthCenter(const glm::vec3 &earthCenter) {
  earthCenter_ = earthCenter;
}
auto AtmosphereSetting::sunIntensity() const -> float { return sunIntensity_; }
void AtmosphereSetting::setSunIntensity(float sunIntensity) {
  sunIntensity_ = sunIntensity;
}
auto AtmosphereSetting::exposure() const -> float { return exposure_; }
void AtmosphereSetting::setExposure(float exposure) { exposure_ = exposure; }
auto AtmosphereSetting::sunAngularRadius() const -> double { return kSunAngularRadius_; }
auto AtmosphereSetting::sunSolidAngle() const -> double { return kSunSolidAngle_; }
auto AtmosphereSetting::lengthUnitInMeters() const -> double {
  return kLengthUnitInMeters_;
}
auto AtmosphereSetting::bottomRadius() const -> double { return kBottomRadius_; }
}