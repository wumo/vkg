#pragma once
#include "vkg/math/glm_common.hpp"
#include "vkg/render/allocation.hpp"

namespace vkg {
class Scene;
struct Lighting {
public: // ref in shaders
  struct Desc {
    uint32_t numLights{0};
    float exposure{4.5f}, gamma{2.2f};
  };
  explicit Lighting(Scene &scene);

  virtual auto numLights() const -> uint32_t;
  virtual auto setNumLights(uint32_t numLights) -> void;
  virtual auto exposure() const -> float;
  virtual auto setExposure(float exposure) -> void;
  virtual auto gamma() const -> float;
  virtual auto setGamma(float gamma) -> void;

protected:
  uint32_t numLights_{0};
  float exposure_{4.5f}, gamma_{2.2f};

  Allocation<Desc> desc;
};

class Light {
public:
  // ref in shaders
  struct alignas(sizeof(glm::vec4)) Desc {
    glm::vec3 color;
    float intensity;
    glm::vec3 location;
    float range;
  };
  Light(Scene &scene, uint32_t id);
  virtual auto id() const -> uint32_t;
  virtual auto color() const -> glm::vec3;
  virtual auto setColor(glm::vec3 color) -> void;
  virtual auto location() const -> glm::vec3;
  virtual auto setLocation(glm::vec3 location) -> void;
  virtual auto intensity() const -> float;
  virtual auto setIntensity(float intensity) -> void;
  virtual auto range() const -> float;
  virtual auto setRange(float range) -> void;

private:
  const uint32_t id_;
  glm::vec3 color_{1.f};
  glm::vec3 location_{0.f, 1.f, 0.f};
  float intensity_{1.f};
  float range_{0};

  Allocation<Desc> desc;
};

}