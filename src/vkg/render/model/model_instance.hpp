#pragma once
#include "transform.hpp"
#include "vkg/render/allocation.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/base/vk_headers.hpp"
#include "vkg/render/shade_model.hpp"
#include "frame_updatable.hpp"

namespace vkg {
class Scene;
class ModelInstance: public FrameUpdatable {
public:
  struct PerFrameRef {
    uint32_t idx;
    uint32_t count;
  };
  struct MeshInstanceDesc {
    /**material buffer offset*/
    PerFrameRef materialDesc;
    /**primitive buffer offset */
    PerFrameRef primitiveDesc;
    /**node transform offset */
    uint32_t nodeTransfIdx;
    /**modelInstance transform offset*/
    PerFrameRef instanceTransf;
    /**whether or not this should be drawn*/
    vk::Bool32 visible;
    /**the draw group used to draw this mesh instance. MeshInsts with same draw group will
     * be grouped*/
    ShadeModel shadeModel;
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
  void updateFrame(uint32_t frameIdx, vk::CommandBuffer commandBuffer) override;

  Scene &scene;
  const uint32_t id_;
  const uint32_t count_;

  uint32_t model_;
  bool visible_{true};
  uint32_t customMatId{nullIdx};

  struct MeshInstInfo {
    ShadeModel shadeModel{ShadeModel::Unknown};
    Allocation<MeshInstanceDesc> desc;
  };
  std::vector<MeshInstInfo> meshInstDescs;

  Transform transform_;

  std::vector<Allocation<Transform>> transfs;
};
}
