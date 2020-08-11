#pragma once
#include "vkg/base/base.hpp"
#include "scene_config.hpp"
#include "model/primitive.hpp"
#include "model/material.hpp"
#include "model/mesh.hpp"
#include "model/node.hpp"
#include "model/model.hpp"
#include "model/model_instance.hpp"
#include "model/light.hpp"
#include "model/camera.hpp"
#include "builder/primitive_builder.hpp"
#include "buffer_allocation.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/math/frustum.hpp"
#include "draw_group.hpp"
#include "model/atmosphere.hpp"
#include <span>

namespace vkg {

class Renderer;

struct ScenePassIn {
  FrameGraphResource<vk::Extent2D> swapchainExtent;
  FrameGraphResource<vk::Format> swapchainFormat;
  FrameGraphResource<uint64_t> swapchainVersion;
};
struct ScenePassOut {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<vk::Rect2D> renderArea;
};

class Scene: public Pass<ScenePassIn, ScenePassOut> {
  friend class Primitive;
  friend class SceneSetupPass;

public:
  Scene(Renderer &renderer, SceneConfig sceneConfig, std::string name);

  auto newPrimitive(
    std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals,
    std::span<Vertex::UV> uvs, std::span<uint32_t> indices, AABB aabb,
    PrimitiveTopology topology = PrimitiveTopology::Triangles,
    DynamicType type = DynamicType::Static) -> uint32_t;
  auto newPrimitives(PrimitiveBuilder &builder) -> std::vector<uint32_t>;
  auto newMaterial(MaterialType type = MaterialType::eNone) -> uint32_t;
  auto newTexture(
    const std::string &imagePath, bool mipmap = true,
    vk::SamplerCreateInfo sampler =
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear},
    const std::string &name = "") -> uint32_t;
  auto newTexture(
    std::span<std::byte> bytes, uint32_t width, uint32_t height, bool mipmap = true,
    vk::SamplerCreateInfo sampler =
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear},
    const std::string &name = "") -> uint32_t;
  auto newMesh(uint32_t primitive, uint32_t material) -> uint32_t;
  auto newNode(const Transform &transform = Transform{}, const std::string &name = "")
    -> uint32_t;
  auto newModel(std::vector<uint32_t> &&nodes, std::vector<Animation> &&animations = {})
    -> uint32_t;
  auto loadModel(const std::string &file, MaterialType materialType = MaterialType::eBRDF)
    -> uint32_t;
  auto newModelInstance(uint32_t model, const Transform &transform = Transform{})
    -> uint32_t;
  auto newLight() -> uint32_t;

  auto camera() -> Camera &;
  auto primitive(uint32_t index) -> Primitive &;
  auto material(uint32_t index) -> Material &;
  auto mesh(uint32_t index) -> Mesh &;
  auto node(uint32_t index) -> Node &;
  auto model(uint32_t index) -> Model &;
  auto modelInstance(uint32_t index) -> ModelInstance &;
  auto texture(uint32_t index) -> Texture &;
  auto light(uint32_t index) -> Light &;
  auto lighting() -> Lighting &;

  auto allocateLightingDesc() -> Allocation<Lighting::Desc>;
  auto allocateLightDesc() -> Allocation<Light::Desc>;
  auto allocateMaterialDesc() -> Allocation<Material::Desc>;
  auto allocateTransform() -> Allocation<Transform>;
  auto allocatePrimitiveDesc() -> Allocation<Primitive::Desc>;
  auto allocateMeshInstDesc() -> Allocation<ModelInstance::MeshInstanceDesc>;

  auto addToDrawGroup(uint32_t meshId, DrawGroup oldGroupID = DrawGroup::Unknown)
    -> DrawGroup;

  auto setup(PassBuilder &builder, const ScenePassIn &inputs) -> ScenePassOut override;
  void compile(Resources &resources) override;

private:
  auto ensureTextures(uint32_t toAdd) const -> void;

  Device &device;
  const FeatureConfig &featureConfig;
  SceneConfig sceneConfig;
  std::string name;

  struct {
    std::unique_ptr<ContiguousAllocation<Vertex::Position>> positions;
    std::unique_ptr<ContiguousAllocation<Vertex::Normal>> normals;
    std::unique_ptr<ContiguousAllocation<Vertex::UV>> uvs;
    std::unique_ptr<ContiguousAllocation<uint32_t>> indices;

    std::unique_ptr<RandomHostAllocation<Primitive::Desc>> primitives;

    std::unique_ptr<RandomHostAllocation<Material::Desc>> materials;
    std::unique_ptr<RandomHostAllocation<Transform>> transforms;
    std::unique_ptr<RandomHostAllocation<ModelInstance::MeshInstanceDesc>> meshInstances;

    std::unique_ptr<RandomHostAllocation<Lighting::Desc>> lighting;
    std::unique_ptr<RandomHostAllocation<Light::Desc>> lights;

    std::unique_ptr<RandomHostAllocation<Camera::Desc>> camera;

    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<vk::DescriptorImageInfo> sampler2Ds;
    uint32_t lastUsedSampler2DIndex{};

    std::unique_ptr<Texture> backImg;
  } Dev;

  struct {
    std::vector<Primitive> primitives;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Node> nodes;
    std::vector<Model> models;
    std::vector<ModelInstance> modelInstances;
    std::unique_ptr<Lighting> lighting;
    std::vector<Light> lights;
    std::unique_ptr<Camera> camera_;
    Atmosphere atmosphere;
    std::vector<uint32_t> drawGroupInstCount;
  } Host;

  vk::Rect2D renderArea;

  ScenePassOut passOut;
  ScenePassIn passIn;
  bool boundPassData{false};
  uint64_t swapchainVersion{0};
};
}
