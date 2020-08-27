#include "material.hpp"
#include "vkg/render/scene.hpp"
namespace vkg {

Material::Material(Scene &scene, uint32_t id, MaterialType type, uint32_t count)
  : scene{scene}, id_{id}, count_{count}, type_{type} {
  errorIf(count == 0, "material count should > 0");
  descs.resize(count);
  for(auto i = 0u; i < count; ++i) {
    descs[i] = scene.allocateMaterialDesc();
    Material::updateDesc(i);
  }
}
auto Material::id() const -> uint32_t { return id_; }
auto Material::count() const -> uint32_t { return count_; }
auto Material::type() const -> MaterialType { return type_; }
auto Material::colorTex() const -> uint32_t { return colorTex_; }
auto Material::pbrTex() const -> uint32_t { return pbrTex_; }
auto Material::normalTex() const -> uint32_t { return normalTex_; }
auto Material::occlusionTex() const -> uint32_t { return occlusionTex_; }
auto Material::emissiveTex() const -> uint32_t { return emissiveTex_; }
auto Material::colorFactor() const -> glm::vec4 { return colorFactor_; }
auto Material::pbrFactor() const -> glm::vec4 { return pbrFactor_; }
auto Material::occlusionStrength() const -> float { return occlusionStrength_; }
auto Material::alphaCutoff() const -> float { return alphaCutoff_; }
auto Material::emissiveFactor() const -> glm::vec4 { return emissiveFactor_; }

auto Material::heightTex() const -> uint32_t { return heightTex_; }

auto Material::descOffset() const -> uint32_t { return descs[0].offset; }
auto Material::setColorFactor(glm::vec4 colorFactor) -> Material & {
  colorFactor_ = colorFactor;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setPbrFactor(glm::vec4 pbrFactor) -> Material & {
  pbrFactor_ = pbrFactor;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setEmissiveFactor(glm::vec4 emissiveFactor) -> Material & {
  emissiveFactor_ = emissiveFactor;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setOcclusionStrength(float occlusionStrength) -> Material & {
  occlusionStrength_ = occlusionStrength;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setAlphaCutoff(float alphaCutoff) -> Material & {
  alphaCutoff_ = alphaCutoff;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setColorTex(uint32_t colorTex) -> Material & {
  colorTex_ = colorTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setPbrTex(uint32_t pbrTex) -> Material & {
  pbrTex_ = pbrTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setNormalTex(uint32_t normalTex) -> Material & {
  normalTex_ = normalTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setOcclusionTex(uint32_t occlusionTex) -> Material & {
  occlusionTex_ = occlusionTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setEmissiveTex(uint32_t emissiveTex) -> Material & {
  emissiveTex_ = emissiveTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
auto Material::setHeightTex(uint32_t heightTex) -> Material & {
  heightTex_ = heightTex;
  scene.scheduleFrameUpdate(Update::Type::Material, id_, count_, ticket);
  return *this;
}
void Material::updateDesc(uint32_t frameIdx) {
  *descs[std::clamp(frameIdx, 0u, count_ - 1)].ptr = {
    colorFactor_,  pbrFactor_,   emissiveFactor_, occlusionStrength_,
    alphaCutoff_,  colorTex_,    pbrTex_,         normalTex_,
    occlusionTex_, emissiveTex_, heightTex_,      static_cast<uint32_t>(type_)};
}
}