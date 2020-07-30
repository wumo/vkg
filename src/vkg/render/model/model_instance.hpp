#pragma once
#include "transform.hpp"
#include "vkg/render/allocation.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/base/vk_headers.hpp"

namespace vkg {
class Scene;
class ModelInstance {
public:
  struct MeshInstanceDesc {
    /**material buffer offset*/
    uint32_t materialDesc;
    /**primitive buffer offset */
    uint32_t primitiveDesc;
    /**node transform offset */
    uint32_t nodeTransf;
    /**modelInstance transform offset*/
    uint32_t instanceTransf;
    /**whether or not this should be drawn*/
    vk::Bool32 visible;
    /**the draw group used to draw this mesh instance. MeshInsts with same draw group will
     * be grouped*/
    uint32_t drawGroupID;
  };
  ModelInstance(Scene &scene, uint32_t id, const Transform &transform, uint32_t modelId);
  virtual auto id() const -> uint32_t;
  virtual auto transform() const -> Transform;
  virtual auto setTransform(const Transform &transform) -> void;
  virtual auto model() const -> uint32_t;
  virtual auto changeModel(uint32_t model) -> void;
  virtual auto visible() const -> bool;
  virtual auto setVisible(bool visible) -> void;
  virtual auto customMaterial() const -> uint32_t;
  virtual auto setCustomMaterial(uint32_t materialId) -> void;

protected:
  Scene &scene;
  const uint32_t id_;
  Transform transform_;
  uint32_t model_;
  bool visible_{true};
  uint32_t customMatId{nullIdx};

  Allocation<Transform> transf;
  std::vector<Allocation<MeshInstanceDesc>> meshInstDescs;
};
}
