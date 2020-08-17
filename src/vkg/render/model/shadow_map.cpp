#include "shadow_map.hpp"
namespace vkg {
auto ShadowMapSetting::isEnabled() const -> bool { return enabled_; }
auto ShadowMapSetting::enable(bool enabled) -> void { enabled_ = enabled; }
auto ShadowMapSetting::numCascades() const -> uint32_t { return numCascades_; }
void ShadowMapSetting::setNumCascades(uint32_t numCascades) {
  numCascades_ = numCascades;
}
auto ShadowMapSetting::textureSize() const -> uint32_t { return textureSize_; }
void ShadowMapSetting::setTextureSize(uint32_t textureSize) {
  textureSize_ = textureSize;
}
auto ShadowMapSetting::far() const -> float { return far_; }
void ShadowMapSetting::setFar(float far) { far_ = far; }
}
