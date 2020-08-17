#pragma once
#include <cstdint>

namespace vkg {
class ShadowMapSetting {
public:
  auto isEnabled() const -> bool;
  auto enable(bool enabled) -> void;
  auto numCascades() const -> uint32_t;
  void setNumCascades(uint32_t numCascades);
  auto textureSize() const -> uint32_t;
  void setTextureSize(uint32_t textureSize);
  auto far() const -> float;
  void setFar(float far);

private:
  bool enabled_{false};
  uint32_t numCascades_{4};
  uint32_t textureSize_{4096};
  float far_{0};
};
}
