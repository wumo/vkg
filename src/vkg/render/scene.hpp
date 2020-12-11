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
#include "shade_model.hpp"
#include "model/atmosphere.hpp"
#include "model/shadow_map.hpp"
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

struct Update {
  enum class Type { Primitive, Material, Light, Instance };
  Type type;
  uint32_t id;
  uint32_t frames;
};

class Scene: public Pass<ScenePassIn, ScenePassOut> {
  friend class Primitive;
  friend class SceneSetupPass;

public:
  Scene(Renderer &renderer, SceneConfig sceneConfig, std::string name);

  auto newPrimitive(
    std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals,
    std::span<Vertex::UV> uvs, std::span<uint32_t> indices, AABB aabb,
    PrimitiveTopology topology = PrimitiveTopology::Triangles, bool perFrame = false)
    -> uint32_t;
  auto newPrimitives(PrimitiveBuilder &builder, bool perFrame = false)
    -> std::vector<uint32_t>;
  auto newMaterial(MaterialType type = MaterialType::eNone, bool perFrame = false)
    -> uint32_t;
  auto newTexture(
    const std::string &imagePath, bool mipmap = true,
    vk::SamplerCreateInfo sampler =
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear},
    const std::string &name = "") -> uint32_t;
  auto newTexture(
    std::span<std::byte> bytes, uint32_t width, uint32_t height,
    vk::Format format = vk::Format::eR8G8B8A8Unorm, bool mipmap = true,
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
  auto loadModel(std::span<std::byte> bytes, MaterialType materialType = MaterialType::eBRDF)
  -> uint32_t;
  auto newModelInstance(
    uint32_t model, const Transform &transform = Transform{}, bool perFrame = false)
    -> uint32_t;
  auto newLight(bool perFrame = false) -> uint32_t;

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
  auto atmosphere() -> AtmosphereSetting &;
  auto shadowmap() -> ShadowMapSetting &;

  auto allocateLightingDesc() const -> Allocation<Lighting::Desc>;
  auto allocateLightDesc() const -> Allocation<Light::Desc>;
  auto allocateMaterialDesc() const -> Allocation<Material::Desc>;
  auto allocateTransform() const -> Allocation<Transform>;
  auto allocatePrimitiveDesc() const -> Allocation<Primitive::Desc>;
  auto allocateMeshInstDesc() const -> Allocation<ModelInstance::MeshInstanceDesc>;

  void scheduleFrameUpdate(
    Update::Type type, uint32_t id, uint32_t frames, uint32_t &ticket);

  auto addToDrawGroup(uint32_t meshId, ShadeModel oldShadeModelID = ShadeModel::Unknown)
    -> ShadeModel;

  void setVisible(ShadeModel shadeModel, bool visible);

  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;

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

    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<vk::DescriptorImageInfo> sampler2Ds;
    uint32_t lastUsedSampler2DIndex{};

    std::vector<std::unique_ptr<Texture>> backImgs;
  } Dev;

  struct {
    std::vector<Primitive> primitives;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Node> nodes;
    std::vector<Model> models;
    std::vector<ModelInstance> modelInstances;
    std::vector<uint32_t> shadeModelCount;
    std::unique_ptr<Lighting> lighting;
    std::vector<Light> lights;
    std::unique_ptr<Camera> camera_;
    AtmosphereSetting atmosphere;
    ShadowMapSetting shadowMap;

    std::vector<Update> updates;
  } Host;

  vk::Rect2D renderArea;

  uint64_t swapchainVersion{0};
};
}
