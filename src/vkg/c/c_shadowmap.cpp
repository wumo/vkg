#include "c_shadowmap.h"
#include "vkg/render/model/shadow_map.hpp"
using namespace vkg;
bool ShadowMapIsEnabled(CShadowMapSetting *shadowmap) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  return shadowmap_->isEnabled();
}
void ShadowMapEnable(CShadowMapSetting *shadowmap, bool enabled) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  shadowmap_->enable(enabled);
}
uint32_t ShadowMapGetNumCascades(CShadowMapSetting *shadowmap) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  return shadowmap_->numCascades();
}
void ShadowMapSetNumCascades(CShadowMapSetting *shadowmap, uint32_t numCascades) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  shadowmap_->setNumCascades(numCascades);
}
uint32_t ShadowMapGetTextureSize(CShadowMapSetting *shadowmap) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  return shadowmap_->textureSize();
}
void ShadowMapSetTextureSize(CShadowMapSetting *shadowmap, uint32_t textureSize) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  shadowmap_->setTextureSize(textureSize);
}
float ShadowMapGetZFar(CShadowMapSetting *shadowmap) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  return shadowmap_->zFar();
}
void ShadowMapSetZFar(CShadowMapSetting *shadowmap, float zFar) {
  auto *shadowmap_ = reinterpret_cast<ShadowMapSetting *>(shadowmap);
  shadowmap_->setZFar(zFar);
}
