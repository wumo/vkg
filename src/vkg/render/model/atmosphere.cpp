#include "atmosphere.hpp"
namespace vkg {

auto Atmosphere::isEnabled() const -> bool { return enabled_; }
auto Atmosphere::enable(bool enabled) -> void { enabled_ = enabled; }
auto Atmosphere::sunDirection() const -> const glm::vec3 & { return sunDirection_; }
void Atmosphere::setSunDirection(const glm::vec3 &sunDirection) {
  sunDirection_ = sunDirection;
}
auto Atmosphere::earthCenter() const -> const glm::vec3 & { return earthCenter_; }
void Atmosphere::setEarthCenter(const glm::vec3 &earthCenter) {
  earthCenter_ = earthCenter;
}
auto Atmosphere::sunIntensity() const -> float { return sunIntensity_; }
void Atmosphere::setSunIntensity(float sunIntensity) { sunIntensity_ = sunIntensity; }
auto Atmosphere::exposure() const -> float { return exposure_; }
void Atmosphere::setExposure(float exposure) { exposure_ = exposure; }
auto Atmosphere::sunAngularRadius() const -> double { return kSunAngularRadius_; }
auto Atmosphere::sunSolidAngle() const -> double { return kSunSolidAngle_; }
auto Atmosphere::lengthUnitInMeters() const -> double { return kLengthUnitInMeters_; }
auto Atmosphere::bottomRadius() const -> double { return kBottomRadius_; }
}