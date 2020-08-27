#pragma once
#include "vkg/math/glm_common.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/render/allocation.hpp"
#include "frame_updatable.hpp"

namespace vkg {
enum class MaterialType : uint32_t {
  /**BRDF without reflection trace*/
  eBRDF = 0x1u,
  eBRDFSG = 0x2u,
  /**BRDF with reflection trace*/
  eReflective = 0x4u,
  /**BRDF with reflection trace and refraction trace*/
  eRefractive = 0x8u,
  /**diffuse coloring*/
  eNone = 0x10u,
  /**transparent color blending*/
  eTransparent = 0x20u,
  /**terrain */
  eTerrain = 0x40u
};

class Scene;
class Material: public FrameUpdatable {
public:
  // ref in shaders
  struct Desc {
    glm::vec4 colorFactor;
    glm::vec4 pbrFactor;
    glm::vec4 emissiveFactor;
    float occlusionStrength;
    float alphaCutoff;
    uint32_t colorTex, pbrTex, normalTex, occlusionTex, emissiveTex, heightTex;
    uint32_t type;
  };

  Material(Scene &scene, uint32_t id, MaterialType type, uint32_t count = 1);
  auto id() const -> uint32_t;
  auto count() const -> uint32_t;
  auto type() const -> MaterialType;
  auto colorTex() const -> uint32_t;
  auto setColorTex(uint32_t colorTex) -> Material &;
  auto pbrTex() const -> uint32_t;
  auto setPbrTex(uint32_t pbrTex) -> Material &;
  auto normalTex() const -> uint32_t;
  auto setNormalTex(uint32_t normalTex) -> Material &;
  auto occlusionTex() const -> uint32_t;
  auto setOcclusionTex(uint32_t occlusionTex) -> Material &;
  auto emissiveTex() const -> uint32_t;
  auto setEmissiveTex(uint32_t emissiveTex) -> Material &;
  auto colorFactor() const -> glm::vec4;
  auto setColorFactor(glm::vec4 colorFactor) -> Material &;
  auto pbrFactor() const -> glm::vec4;
  auto setPbrFactor(glm::vec4 pbrFactor) -> Material &;
  auto occlusionStrength() const -> float;
  auto setOcclusionStrength(float occlusionStrength) -> Material &;
  auto alphaCutoff() const -> float;
  auto setAlphaCutoff(float alphaCutoff) -> Material &;
  auto emissiveFactor() const -> glm::vec4;
  auto setEmissiveFactor(glm::vec4 emissiveFactor) -> Material &;
  auto heightTex() const -> uint32_t;
  auto setHeightTex(uint32_t heightTex) -> Material &;

  auto descOffset() const -> uint32_t;

protected:
  void updateDesc(uint32_t frameIdx) override;

protected:
  Scene &scene;
  const uint32_t id_;
  const uint32_t count_;
  const MaterialType type_;

  glm::vec4 colorFactor_{1.f};
  glm::vec4 pbrFactor_{0.f, 1.f, 0.f, 0.f};
  glm::vec4 emissiveFactor_{0.f, 0.f, 0.f, 0.f};
  float occlusionStrength_{1.f};
  float alphaCutoff_{0.f};
  uint32_t colorTex_{nullIdx}, pbrTex_{nullIdx}, normalTex_{nullIdx},
    occlusionTex_{nullIdx}, emissiveTex_{nullIdx}, heightTex_{nullIdx};

  std::vector<Allocation<Desc>> descs;
};

}