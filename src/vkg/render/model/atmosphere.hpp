#pragma once
#include "vkg/math/glm_common.hpp"

namespace vkg {
class AtmosphereSetting {

public:
  auto isEnabled() const -> bool;
  auto enable(bool enabled) -> void;

  auto sunDirection() const -> const glm::vec3 &;
  void setSunDirection(const glm::vec3 &sunDirection);
  auto earthCenter() const -> const glm::vec3 &;
  void setEarthCenter(const glm::vec3 &earthCenter);
  auto sunIntensity() const -> float;
  void setSunIntensity(float sunIntensity);
  auto exposure() const -> float;
  void setExposure(float exposure);

  auto sunAngularRadius() const -> double;
  auto sunSolidAngle() const -> double;
  auto lengthUnitInMeters() const -> double;
  auto bottomRadius() const -> double;

  const double kSunAngularRadius_{0.00935 / 2.0};
  const double kSunSolidAngle_{
    glm::pi<double>() * kSunAngularRadius_ * kSunAngularRadius_};
  const double kLengthUnitInMeters_{1.0};
  const double kBottomRadius_{6360000.0};
  const double kTopRadius{6420000.0};

private:
  glm::vec3 sunDirection_{0, 1, 0};
  glm::vec3 earthCenter_{0, -kBottomRadius_ / kLengthUnitInMeters_, 0};
  float sunIntensity_{1};
  float exposure_{10.0};
  bool enabled_{true};
};
}
