#include "scene.hpp"
#include "renderer.hpp"
#include "vkg/render/builder/gltf_loader.hpp"
#include <utility>

namespace vkg {
Scene::Scene(Renderer &renderer, SceneConfig sceneConfig, std::string name)
  : device{renderer.device()},
    featureConfig{renderer.featureConfig()},
    sceneConfig{sceneConfig},
    name{std::move(name)} {

  renderArea = vk::Rect2D{
    {sceneConfig.offsetX, sceneConfig.offsetY},
    {sceneConfig.extentW, sceneConfig.extentH}};

  Dev.positions = std::make_unique<ContiguousAllocation<Vertex::Position>>(
    buffer::devVertexBuffer, device, sceneConfig.maxNumVertices, "positions");
  Dev.normals = std::make_unique<ContiguousAllocation<Vertex::Normal>>(
    buffer::devVertexBuffer, device, sceneConfig.maxNumVertices, "normals");
  Dev.uvs = std::make_unique<ContiguousAllocation<Vertex::UV>>(
    buffer::devVertexBuffer, device, sceneConfig.maxNumVertices, "uvs");
  Dev.indices = std::make_unique<ContiguousAllocation<uint32_t>>(
    buffer::devIndexBuffer, device, sceneConfig.maxNumIndices, "indices");
  Dev.primitives = std::make_unique<RandomHostAllocation<Primitive::Desc>>(
    buffer::hostStorageBuffer, device, sceneConfig.maxNumPrimitives, "primitives");
  Dev.materials = std::make_unique<RandomHostAllocation<Material::Desc>>(
    buffer::hostStorageBuffer, device, sceneConfig.maxNumMaterials, "materials");
  Dev.transforms = std::make_unique<RandomHostAllocation<Transform>>(
    buffer::hostStorageBuffer, device, sceneConfig.maxNumTransforms, "transforms");
  Dev.meshInstances =
    std::make_unique<RandomHostAllocation<ModelInstance::MeshInstanceDesc>>(
      buffer::hostStorageBuffer, device, sceneConfig.maxNumMeshInstances,
      "meshInstances");
  Dev.lighting = std::make_unique<RandomHostAllocation<Lighting::Desc>>(
    buffer::hostUniformBuffer, device, 1, "lighting");
  Host.lighting = std::make_unique<Lighting>(*this);
  Dev.lights = std::make_unique<RandomHostAllocation<Light::Desc>>(
    buffer::hostStorageBuffer, device, sceneConfig.maxNumLights, "lights");
  Host.camera_ = std::make_unique<Camera>(glm::vec3{10, 10, 10}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0},
    glm::radians(45.f), 0.1f, 1000.f, 1, 1);

  Dev.textures.push_back(image::makeSampler2DTex("empty tex", device, 1, 1));
  glm::vec4 color{1.f, 1.f, 1.f, 1.f};
  image::upload(
    *Dev.textures.back(), {reinterpret_cast<std::byte *>(&color), sizeof(color)});
  Dev.textures.back()->setSampler({});

  Dev.sampler2Ds.reserve(sceneConfig.maxNumTextures);
  auto i = 0u;
  for(; i < Dev.textures.size(); ++i)
    Dev.sampler2Ds.emplace_back(
      Dev.textures[i]->sampler(), Dev.textures[i]->imageView(),
      Dev.textures[i]->layout());
  // bind unused textures to empty texture;
  for(; i < sceneConfig.maxNumTextures; ++i)
    Dev.sampler2Ds.emplace_back(
      Dev.textures[0]->sampler(), Dev.textures[0]->imageView(),
      Dev.textures[0]->layout());

  Dev.lastUsedSampler2DIndex = uint32_t(Dev.textures.size());

  newMaterial(MaterialType::eNone);

  Host.drawGroupInstCount.resize(value(DrawGroup::Last) + 1);
}

auto Scene::newPrimitive(
  std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals,
  std::span<Vertex::UV> uvs, std::span<uint32_t> indices, AABB aabb,
  PrimitiveTopology topology, DynamicType type) -> uint32_t {
  auto pos = Dev.positions->add(positions);
  auto normal = Dev.normals->add(normals);
  auto uv = Dev.uvs->add(uvs);
  auto index = Dev.indices->add(indices);
  auto id = uint32_t(Host.primitives.size());
  Host.primitives.emplace_back(*this, id, index, pos, normal, uv, topology, type);
  Host.primitives.back().setAABB(aabb);
  return id;
}

auto Scene::newPrimitives(PrimitiveBuilder &builder) -> std::vector<uint32_t> {
  std::vector<uint32_t> primitives;
  for(auto &p: builder.primitives()) {
    auto id = newPrimitive(
      builder.positions().subspan(p.position.start, p.position.size),
      builder.normals().subspan(p.normal.start, p.normal.size),
      builder.uvs().subspan(p.uv.start, p.uv.size),
      builder.indices().subspan(p.index.start, p.index.size), p.aabb, p.topology, p.type);
    primitives.emplace_back(id);
  }
  return primitives;
}
auto Scene::newMaterial(MaterialType type) -> uint32_t {
  auto id = uint32_t(Host.materials.size());
  Host.materials.emplace_back(*this, id, type);
  return id;
}
auto Scene::ensureTextures(uint32_t toAdd) const -> void {
  errorIf(
    Dev.textures.size() + toAdd > sceneConfig.maxNumTextures,
    "exceeding maximum number of textures!");
}
auto Scene::newTexture(
  const std::string &imagePath, bool mipmap, vk::SamplerCreateInfo sampler,
  const std::string &name) -> uint32_t {
  ensureTextures(1);
  Dev.textures.push_back(image::load2DFromFile(name, device, imagePath, mipmap));
  Dev.textures.back()->setSampler(sampler);
  Dev.sampler2Ds[Dev.textures.size() - 1] = {
    Dev.textures.back()->sampler(), Dev.textures.back()->imageView(),
    Dev.textures.back()->layout()};
  return uint32_t(Dev.textures.size() - 1);
}
auto Scene::newTexture(
  std::span<std::byte> bytes, uint32_t width, uint32_t height, bool mipmap,
  vk::SamplerCreateInfo sampler, const std::string &name) -> uint32_t {
  ensureTextures(1);
  Dev.textures.push_back(
    image::load2DFromBytes(name, device, bytes, width, height, mipmap));
  Dev.textures.back()->setSampler(sampler);
  Dev.sampler2Ds[Dev.textures.size() - 1] = {
    Dev.textures.back()->sampler(), Dev.textures.back()->imageView(),
    Dev.textures.back()->layout()};
  return uint32_t(Dev.textures.size() - 1);
}
auto Scene::newMesh(uint32_t primitive, uint32_t material) -> uint32_t {
  auto id = uint32_t(Host.meshes.size());
  Host.meshes.emplace_back(id, primitive, material);
  return id;
}
auto Scene::newNode(const Transform &transform, const std::string &name) -> uint32_t {
  auto id = uint32_t(Host.nodes.size());
  Host.nodes.emplace_back(*this, id, transform);
  Host.nodes.back().setName(name);
  return id;
}
auto Scene::newModel(std::vector<uint32_t> &&nodes, std::vector<Animation> &&animations)
  -> uint32_t {
  auto id = uint32_t(Host.models.size());
  Host.models.emplace_back(*this, id, std::move(nodes), std::move(animations));
  return id;
}
auto Scene::loadModel(const std::string &file, MaterialType materialType) -> uint32_t {
  GLTFLoader loader{*this, materialType};
  return loader.load(file);
}
auto Scene::newModelInstance(uint32_t model, const Transform &transform) -> uint32_t {
  auto id = uint32_t(Host.modelInstances.size());
  Host.modelInstances.emplace_back(*this, id, transform, model);
  return id;
}
auto Scene::newLight() -> uint32_t {
  auto id = uint32_t(Host.lights.size());
  Host.lights.emplace_back(*this, id);
  Host.lighting->setNumLights(Host.lighting->numLights() + 1);
  return id;
}
auto Scene::camera() -> Camera & { return *Host.camera_; }
auto Scene::primitive(uint32_t index) -> Primitive & { return Host.primitives.at(index); }
auto Scene::material(uint32_t index) -> Material & { return Host.materials.at(index); }
auto Scene::mesh(uint32_t index) -> Mesh & { return Host.meshes.at(index); }
auto Scene::node(uint32_t index) -> Node & { return Host.nodes.at(index); }
auto Scene::model(uint32_t index) -> Model & { return Host.models.at(index); }
auto Scene::modelInstance(uint32_t index) -> ModelInstance & {
  return Host.modelInstances.at(index);
}
auto Scene::texture(uint32_t index) -> Texture & { return *Dev.textures.at(index); }
auto Scene::light(uint32_t index) -> Light & { return Host.lights.at(index); }
auto Scene::lighting() -> Lighting & { return *Host.lighting; }

auto Scene::atmosphere() -> AtmosphereSetting & { return Host.atmosphere; }

auto Scene::allocateLightingDesc() -> Allocation<Lighting::Desc> {
  return Dev.lighting->allocate();
}
auto Scene::allocateLightDesc() -> Allocation<Light::Desc> {
  return Dev.lights->allocate();
}
auto Scene::allocateMaterialDesc() -> Allocation<Material::Desc> {
  return Dev.materials->allocate();
}
auto Scene::allocateTransform() -> Allocation<Transform> {
  return Dev.transforms->allocate();
}
auto Scene::allocatePrimitiveDesc() -> Allocation<Primitive::Desc> {
  return Dev.primitives->allocate();
}
auto Scene::allocateMeshInstDesc() -> Allocation<ModelInstance::MeshInstanceDesc> {
  return Dev.meshInstances->allocate();
}
auto Scene::addToDrawGroup(uint32_t meshId, DrawGroup oldGroupID) -> DrawGroup {
  auto &mesh_ = mesh(meshId);
  auto &primitive_ = primitive(mesh_.primitive());
  auto &material_ = material(mesh_.material());

  DrawGroup gID{};
  switch(primitive_.topology()) {
    case PrimitiveTopology::Triangles:
      switch(material_.type()) {
        case MaterialType::eNone: gID = DrawGroup::Unlit; break;
        case MaterialType::eBRDF:
        case MaterialType::eBRDFSG: gID = DrawGroup::BRDF; break;
        case MaterialType::eReflective: gID = DrawGroup::Reflective; break;
        case MaterialType::eRefractive: gID = DrawGroup::Refractive; break;
        case MaterialType::eTransparent: gID = DrawGroup::Transparent; break;
        case MaterialType::eTerrain: gID = DrawGroup::Terrain; break;
      }
      break;
    case PrimitiveTopology::Lines:
      switch(material_.type()) {
        case MaterialType::eRefractive:
        case MaterialType::eTransparent: gID = DrawGroup::TransparentLines; break;
        default: gID = DrawGroup::OpaqueLines;
      }
      break;
    case PrimitiveTopology::Procedural:
    case PrimitiveTopology::Patches: throw std::runtime_error("Not supported"); break;
  }
  if(oldGroupID != DrawGroup::Unknown) Host.drawGroupInstCount[value(oldGroupID)]--;
  Host.drawGroupInstCount[value(gID)]++;
  return gID;
}
auto Scene::shadowmap() -> ShadowMapSetting & { return Host.shadowMap; }
}
