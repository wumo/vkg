#pragma once
#include "vkg/math/glm_common.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/render/allocation.hpp"

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
class Material {
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

  Material(Scene &scene, uint32_t id, MaterialType type);
  virtual auto id() const -> uint32_t;
  virtual auto colorTex() const -> uint32_t;
  virtual auto setColorTex(uint32_t colorTex) -> Material &;
  virtual auto pbrTex() const -> uint32_t;
  virtual auto setPbrTex(uint32_t pbrTex) -> Material &;
  virtual auto normalTex() const -> uint32_t;
  virtual auto setNormalTex(uint32_t normalTex) -> Material &;
  virtual auto occlusionTex() const -> uint32_t;
  virtual auto setOcclusionTex(uint32_t occlusionTex) -> Material &;
  virtual auto emissiveTex() const -> uint32_t;
  virtual auto setEmissiveTex(uint32_t emissiveTex) -> Material &;
  virtual auto colorFactor() const -> glm::vec4;
  virtual auto setColorFactor(glm::vec4 colorFactor) -> Material &;
  virtual auto pbrFactor() const -> glm::vec4;
  virtual auto setPbrFactor(glm::vec4 pbrFactor) -> Material &;
  virtual auto occlusionStrength() const -> float;
  virtual auto setOcclusionStrength(float occlusionStrength) -> Material &;
  virtual auto alphaCutoff() const -> float;
  virtual auto setAlphaCutoff(float alphaCutoff) -> Material &;
  virtual auto emissiveFactor() const -> glm::vec4;
  virtual auto setEmissiveFactor(glm::vec4 emissiveFactor) -> Material &;
  virtual auto heightTex() const -> uint32_t;
  virtual auto setHeightTex(uint32_t heightTex) -> Material &;
  virtual auto type() const -> MaterialType;
  virtual auto descOffset() const -> uint32_t;

protected:
  const uint32_t id_;

  glm::vec4 colorFactor_{1.f};
  glm::vec4 pbrFactor_{0.f, 1.f, 0.f, 0.f};
  glm::vec4 emissiveFactor_{0.f, 0.f, 0.f, 0.f};
  float occlusionStrength_{1.f};
  float alphaCutoff_{0.f};
  uint32_t colorTex_{nullIdx}, pbrTex_{nullIdx}, normalTex_{nullIdx},
    occlusionTex_{nullIdx}, emissiveTex_{nullIdx}, heightTex_{nullIdx};
  const MaterialType type_;

  Allocation<Desc> desc;
};

}