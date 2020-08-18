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
  auto zFar() const -> float;
  void setZFar(float far);

private:
  bool enabled_{false};
  uint32_t numCascades_{4};
  uint32_t textureSize_{4096};
  float zfar_{0};
};
}
