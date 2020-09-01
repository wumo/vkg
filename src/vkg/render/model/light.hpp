#pragma once
#include "vkg/math/glm_common.hpp"
#include "vkg/render/allocation.hpp"
#include "frame_updatable.hpp"

namespace vkg {
class Scene;
struct Lighting {
public: // ref in shaders
  struct Desc {
    uint32_t numLights{0};
    float exposure{4.5f}, gamma{2.2f};
  };
  explicit Lighting(Scene &scene);

  auto numLights() const -> uint32_t;
  auto setNumLights(uint32_t numLights) -> void;
  auto exposure() const -> float;
  auto setExposure(float exposure) -> void;
  auto gamma() const -> float;
  auto setGamma(float gamma) -> void;

protected:
  uint32_t numLights_{0};
  float exposure_{4.5f}, gamma_{2.2f};

  Allocation<Desc> desc;
};

class Light: public FrameUpdatable {
public:
  // ref in shaders
  struct alignas(sizeof(glm::vec4)) Desc {
    glm::vec3 color;
    float intensity;
    glm::vec3 location;
    float range;
  };
  Light(Scene &scene, uint32_t id, uint32_t count = 1);
  auto id() const -> uint32_t;
  auto count() const -> uint32_t;
  auto color() const -> glm::vec3;
  auto setColor(glm::vec3 color) -> void;
  auto location() const -> glm::vec3;
  auto setLocation(glm::vec3 location) -> void;
  auto intensity() const -> float;
  auto setIntensity(float intensity) -> void;
  auto range() const -> float;
  auto setRange(float range) -> void;

protected:
  void updateFrame(uint32_t frameIdx, vk::CommandBuffer commandBuffer) override;

private:
  Scene &scene;
  const uint32_t id_;
  const uint32_t count_;

  glm::vec3 color_{1.f};
  float intensity_{1.f};
  glm::vec3 location_{0.f, 1.f, 0.f};
  float range_{0};

  std::vector<Allocation<Desc>> descs;
};

}