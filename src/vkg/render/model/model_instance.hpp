#pragma once
#include "transform.hpp"
#include "vkg/render/allocation.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/base/vk_headers.hpp"
#include "vkg/render/draw_group.hpp"
#include "frame_updatable.hpp"

namespace vkg {
class Scene;
class ModelInstance: public FrameUpdatable {
public:
  struct MeshInstanceDesc {
    /**material buffer offset*/
    uint32_t materialDescIdx, materialCount;
    /**primitive buffer offset */
    uint32_t primitiveDescIdx, primitiveCount;
    /**node transform offset */
    uint32_t nodeTransfIdx;
    /**modelInstance transform offset*/
    uint32_t instanceTransf, instanceTransfCount;
    /**whether or not this should be drawn*/
    vk::Bool32 visible;
    /**the draw group used to draw this mesh instance. MeshInsts with same draw group will
     * be grouped*/
    DrawGroup drawGroupID;
  };
  ModelInstance(
    Scene &scene, uint32_t id, const Transform &transform, uint32_t modelId,
    uint32_t count = 1);
  auto id() const -> uint32_t;
  auto count() const -> uint32_t;
  auto transform() const -> Transform;
  auto setTransform(const Transform &transform) -> void;
  auto model() const -> uint32_t;
  auto changeModel(uint32_t model) -> void;
  auto visible() const -> bool;
  auto setVisible(bool visible) -> void;
  auto customMaterial() const -> uint32_t;
  auto setCustomMaterial(uint32_t materialId) -> void;

protected:
  void updateDesc(uint32_t frameIdx) override;

protected:
  Scene &scene;
  const uint32_t id_;
  const uint32_t count_;

  uint32_t model_;
  bool visible_{true};
  uint32_t customMatId{nullIdx};

  struct MeshInstInfo {
    DrawGroup drawGroupID{DrawGroup::Unknown};
    Allocation<MeshInstanceDesc> desc;
  };
  std::vector<MeshInstInfo> meshInstDescs;

  Transform transform_;

  std::vector<Allocation<Transform>> transfs;
};
}
