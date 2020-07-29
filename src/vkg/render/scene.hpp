#pragma once
#include "vkg/base/base.hpp"
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
#include <span>

namespace vkg {
struct SceneConfig {
  /** renderArea */
  int32_t offsetX{0}, offsetY{0};
  uint32_t extentW{0}, extentH{0};

  /**max number of vertices and indices*/
  uint32_t maxNumVertices{1000'0000}, maxNumIndices{1000'0000};
  /**max number of model instances*/
  uint32_t maxNumTransforms{10'0000};
  /**max number of materials*/
  uint32_t maxNumMaterials{1'0000};
  /**max number of mesh instances*/
  uint32_t maxNumPrimitives{100'0000};
  uint32_t maxNumMeshInstances{100'0000};
  uint32_t maxNumOpaqueTriangles{100'0000};
  uint32_t maxNumOpaqueLines{1000};
  uint32_t maxNumTransparentTriangles{1000};
  uint32_t maxNumTransparentLines{1000};
  /**max number of texture including 2d and cube map.*/
  uint32_t maxNumTextures{1000};
  /**max number of lights*/
  uint32_t maxNumLights{1};
  uint32_t numCascades{4};

  uint32_t sampleCount{1};
};

struct ScenePassOut {
  FrameGraphResource positions;
  FrameGraphResource normals;
  FrameGraphResource uvs;
  FrameGraphResource indices;
  FrameGraphResource primitives;
  FrameGraphResource materials;
  FrameGraphResource transforms;
  FrameGraphResource meshInstances;
  FrameGraphResource matrices;
  FrameGraphResource lighting;
  FrameGraphResource lights;
  FrameGraphResource camera;
};

class Scene: public PassDef {
public:
  Scene(Device &device, SceneConfig &sceneConfig, std::string name);

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
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear})
    -> uint32_t;
  auto newTexture(
    std::span<std::byte> bytes, uint32_t width, uint32_t height, bool mipmap = true,
    vk::SamplerCreateInfo sampler =
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear})
    -> uint32_t;
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

  auto resize(uint32_t width, uint32_t height) -> void;

  void setup(PassBuilder &builder) override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

  auto render() -> void;

private:
  auto ensureTextures(uint32_t toAdd) const -> void;

  Device &device;
  SceneConfig &sceneConfig;
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
    std::unique_ptr<RandomHostAllocation<glm::mat4>> matrices;

    std::unique_ptr<RandomHostAllocation<Lighting::Desc>> lighting;
    std::unique_ptr<RandomHostAllocation<Light::Desc>> lights;

    std::unique_ptr<RandomHostAllocation<Camera::Desc>> camera;

    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<vk::DescriptorImageInfo> sampler2Ds;
    uint32_t lastUsedSampler2DIndex{};
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
  } Host;

  vk::Rect2D renderArea;

  ScenePassOut out;
  bool boundPassData{false};
};
}
