#include "gltf_loader.hpp"
#include "vkg/render/scene.hpp"
#include <stb_image.h>
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {
using namespace glm;

GLTFLoader::GLTFLoader(Scene &scene, MaterialType materialType)
  : scene{scene}, defaultMatType{materialType} {}
auto GLTFLoader::load(const std::string &file) -> uint32_t {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err, warn;
  auto result = endWith(file, ".gltf") ?
                  loader.LoadASCIIFromFile(&model, &err, &warn, file) :
                  loader.LoadBinaryFromFile(&model, &err, &warn, file);
  errorIf(!result, "failed to load glTF ", file);

  loadTextureSamplers(model);
  loadTextures(model);
  loadMaterials(model);

  std::vector<uint32_t> nodes;
  const auto &_scene = model.scenes[std::max(model.defaultScene, 0)];
  _nodes.resize(model.nodes.size());
  for(int i: _scene.nodes)
    nodes.push_back(loadNode(i, model));

  loadAnimations(model);
  return scene.newModel(std::move(nodes), std::move(animations));
}

auto getVkWrapMode(int32_t wrapMode) -> vk::SamplerAddressMode {
  switch(wrapMode) {
    case 10497: return vk::SamplerAddressMode::eRepeat;
    case 33071: return vk::SamplerAddressMode::eClampToEdge;
    case 33648: return vk::SamplerAddressMode ::eMirroredRepeat;
    default:
      error("not supported SamplerAddressMode", wrapMode);
      return vk::SamplerAddressMode::eMirroredRepeat;
  }
}

auto getVkFilterMode(int32_t filterMode) -> vk::Filter {
  switch(filterMode) {
    case 9728: return vk::Filter::eNearest;
    case 9729: return vk::Filter::eLinear;
    case 9984: return vk::Filter::eNearest;
    case 9985: return vk::Filter::eNearest;
    case 9986: return vk::Filter::eLinear;
    case 9987: return vk::Filter::eLinear;
    case -1: return vk::Filter::eLinear;
    default: error("not supported Filter", filterMode); return vk::Filter::eLinear;
  }
}

void GLTFLoader::loadTextureSamplers(const tinygltf::Model &model) {
  for(auto &sampler: model.samplers) {
    vk::SamplerCreateInfo info{
      {},
      getVkFilterMode(sampler.magFilter),
      getVkFilterMode(sampler.minFilter),
      vk::SamplerMipmapMode::eLinear,
      getVkWrapMode(sampler.wrapS),
      getVkWrapMode(sampler.wrapT),
      getVkWrapMode(sampler.wrapT)};
    samplerDefs.push_back(info);
  }
}
//
void GLTFLoader::loadTextures(const tinygltf::Model &model) {
  for(auto &tex: model.textures) {
    auto &image = model.images[tex.source];
    auto size = image.width * image.height * 4u;
    auto sampler =
      tex.sampler == -1 ?
        vk::SamplerCreateInfo{
          {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear} :
        samplerDefs[tex.sampler];
    auto dataPtr = UniqueConstBytes(image.image.data(), [](const unsigned char *ptr) {});
    auto channel = image.component;
    if(channel != 4) {
      int w, h;
      dataPtr = UniqueConstBytes(
        stbi_load_from_memory(
          image.image.data(), int(image.image.size()), &w, &h, &channel, STBI_rgb_alpha),
        [](const unsigned char *ptr) {
          stbi_image_free(const_cast<unsigned char *>(ptr));
        });
    }
    textures.push_back(scene.newTexture(
      {(std::byte *)(dataPtr.get()), size}, image.width, image.height,
      vk::Format::eR8G8B8A8Unorm, true, sampler));
  }
}

void GLTFLoader::loadMaterials(const tinygltf::Model &model) {
  for(const auto &mat: model.materials) {
    MaterialType type{defaultMatType};
    float alphaCutoff{0.f};
    if(mat.additionalValues.contains("alphaMode")) {
      auto alphaMode = mat.additionalValues.at("alphaMode").string_value;
      if(alphaMode != "OPAQUE") type = MaterialType::eTransparent;
      if(alphaMode == "MASK") {
        alphaCutoff = 0.5f;
        if(mat.additionalValues.contains("alphaCutoff"))
          alphaCutoff = float(mat.additionalValues.at("alphaCutoff").number_value);
      }
    }
    if(
      defaultMatType != MaterialType::eNone &&
      mat.extensions.contains("KHR_materials_pbrSpecularGlossiness"))
      type = MaterialType ::eBRDFSG;
    auto materialId = scene.newMaterial(type);
    auto &material = scene.material(materialId);

    material.setAlphaCutoff(alphaCutoff);

    if(mat.values.contains("baseColorFactor"))
      material.setColorFactor(
        make_vec4(mat.values.at("baseColorFactor").ColorFactor().data()));
    if(mat.values.contains("baseColorTexture"))
      material.setColorTex(textures.at(mat.values.at("baseColorTexture").TextureIndex()));

    if(mat.values.contains("metallicRoughnessTexture"))
      material.setPbrTex(
        textures.at(mat.values.at("metallicRoughnessTexture").TextureIndex()));
    vec4 pbrFactor{1.f, 1.f, 1.f, 1.f};
    if(mat.values.contains("roughnessFactor"))
      pbrFactor.g = static_cast<float>(mat.values.at("roughnessFactor").Factor());
    if(mat.values.contains("metallicFactor"))
      pbrFactor.b = static_cast<float>(mat.values.at("metallicFactor").Factor());
    material.setPbrFactor(pbrFactor);

    if(mat.additionalValues.contains("normalTexture"))
      material.setNormalTex(
        textures.at(mat.additionalValues.at("normalTexture").TextureIndex()));

    if(mat.additionalValues.contains("emissiveTexture"))
      material.setEmissiveTex(
        textures.at(mat.additionalValues.at("emissiveTexture").TextureIndex()));
    if(mat.additionalValues.contains("emissiveFactor")) {
      material.setEmissiveFactor(vec4(
        make_vec3(mat.additionalValues.at("emissiveFactor").ColorFactor().data()), 1.f));
    }

    if(mat.additionalValues.contains("occlusionTexture"))
      material.setOcclusionTex(
        textures.at(mat.additionalValues.at("occlusionTexture").TextureIndex()));

    if(mat.extensions.contains("KHR_materials_pbrSpecularGlossiness")) {
      auto ext = mat.extensions.at("KHR_materials_pbrSpecularGlossiness");
      if(ext.Has("specularGlossinessTexture")) {
        auto index = ext.Get("specularGlossinessTexture").Get("index").Get<int>();
        material.setPbrTex(textures.at(index));
      }
      if(ext.Has("diffuseTexture")) {
        auto index = ext.Get("diffuseTexture").Get("index").Get<int>();
        material.setColorTex(textures.at(index));
      }
      if(ext.Has("diffuseFactor")) {
        auto factor = ext.Get("diffuseFactor");
        vec4 diffuse{1.f};
        for(int i = 0; i < factor.ArrayLen(); ++i) {
          auto val = factor.Get(i);
          diffuse[i] = val.IsNumber() ? float(val.Get<double>()) : float(val.Get<int>());
        }
        material.setColorFactor(diffuse);
      }
      if(ext.Has("specularFactor")) {
        auto factor = ext.Get("specularFactor");
        vec4 specular{1.f};
        for(int i = 0; i < factor.ArrayLen(); ++i) {
          auto val = factor.Get(i);
          specular[i] = val.IsNumber() ? float(val.Get<double>()) : float(val.Get<int>());
        }
        material.setPbrFactor(specular);
      }
      if(ext.Has("glossinessFactor")) {
        auto glossiness = float(ext.Get("glossinessFactor").Get<double>());
        auto specular = material.pbrFactor();
        specular.w = glossiness;
        material.setColorFactor(specular);
      }
    }

    materials.push_back(materialId);
  }
}

auto GLTFLoader::loadNode(int thisID, const tinygltf::Model &model) -> uint32_t {
  const auto &node = model.nodes[thisID];
  Transform t;
  if(node.matrix.size() == 16) {
    auto m = make_mat4x4(node.matrix.data());
    t = Transform{m};
  } else {
    t.translation = node.translation.size() != 3 ? dvec3{0.} :
                                                   make_vec3(node.translation.data());
    t.scale = node.scale.size() != 3 ? dvec3{1.f} : make_vec3(node.scale.data());
    t.rotation = node.rotation.size() != 4 ? dquat{1, 0, 0, 0} :
                                             make_quat(node.rotation.data());
  }
  auto nodeId = scene.newNode(t, node.name);
  _nodes[thisID] = nodeId;

  if(node.mesh > -1) {
    const auto &mesh = model.meshes[node.mesh];
    for(const auto &primitive: mesh.primitives)
      scene.node(nodeId).addMeshes({loadPrimitive(model, primitive)});
  }
  if(!node.children.empty())
    for(auto childID: node.children) {
      auto child = loadNode(childID, model);
      scene.node(nodeId).addChildren({child});
    }
  return nodeId;
}

auto GLTFLoader::loadPrimitive(
  const tinygltf::Model &model, const tinygltf::Primitive &primitive) -> uint32_t {
  errorIf(primitive.mode != 4, "model primitive mode isn't triangles!");

  auto aabb = loadVertices(model, primitive);
  loadIndices(model, primitive);
  auto _primitive = scene.newPrimitive(positions, normals, uvs, indices, aabb);
  auto material = primitive.material < 0 ? 0 : materials.at(primitive.material);
  return scene.newMesh(_primitive, material);
}

AABB GLTFLoader::loadVertices(
  const tinygltf::Model &model, const tinygltf::Primitive &primitive) {
  errorIf(!primitive.attributes.contains("POSITION"), "missing required POSITION data!");

  positions.clear();
  normals.clear();
  uvs.clear();

  auto verticesID = primitive.attributes.at("POSITION");

  glm::vec3 posMin{};
  glm::vec3 posMax{};
  const float *bufferPos = nullptr;
  const float *bufferNormals = nullptr;
  const float *bufferTexCoords = nullptr;

  uint32_t posByteStride = 0;
  uint32_t normByteStride = 0;
  uint32_t uv0ByteStride = 0;

  auto posAccessor = model.accessors[verticesID];

  positions.reserve(posAccessor.count);
  normals.reserve(posAccessor.count);
  uvs.reserve(posAccessor.count);

  const auto &posView = model.bufferViews[posAccessor.bufferView];
  bufferPos = (const float *)(&(
    model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
  posMin =
    vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
  posMax =
    vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
  posByteStride = posAccessor.ByteStride(posView) ?
                    (posAccessor.ByteStride(posView) / uint32_t(sizeof(float))) :
                    tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3) *
                      tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT);

  if(primitive.attributes.contains("NORMAL")) {
    const auto &normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
    const auto &normView = model.bufferViews[normAccessor.bufferView];
    bufferNormals =
      (const float *)(&(model.buffers[normView.buffer]
                          .data[normAccessor.byteOffset + normView.byteOffset]));
    normByteStride = normAccessor.ByteStride(normView) ?
                       (normAccessor.ByteStride(normView) / uint32_t(sizeof(float))) :
                       tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3) *
                         tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT);
  }

  if(primitive.attributes.contains("TEXCOORD_0")) {
    const auto &uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
    const auto &uvView = model.bufferViews[uvAccessor.bufferView];
    bufferTexCoords = (const float *)(&(
      model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
    uv0ByteStride = uvAccessor.ByteStride(uvView) ?
                      (uvAccessor.ByteStride(uvView) / uint32_t(sizeof(float))) :
                      tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2) *
                        tinygltf::GetComponentSizeInBytes(TINYGLTF_COMPONENT_TYPE_FLOAT);
  }

  //vertices
  for(size_t v = 0; v < posAccessor.count; v++) {
    positions.push_back(make_vec3(&bufferPos[v * posByteStride]));
    normals.push_back(
      bufferNormals ? make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3{});
    uvs.push_back(
      bufferTexCoords ? make_vec2(&bufferTexCoords[v * uv0ByteStride]) : glm::vec2{});
  }
  return {posMin, posMax};
}

void GLTFLoader::loadIndices(
  const tinygltf::Model &model, const tinygltf::Primitive &primitive) {
  indices.clear();

  if(primitive.indices < 0) {
    for(int i = 0; i < positions.size(); ++i)
      indices.push_back(i);
    return;
  }

  auto indicesID = primitive.indices;

  auto &indicesAccessor = model.accessors[indicesID];
  auto &indicesView = model.bufferViews[indicesAccessor.bufferView];
  auto &buffer = model.buffers[indicesView.buffer];
  auto buf = &buffer.data[indicesAccessor.byteOffset + indicesView.byteOffset];

  indices.reserve(indicesAccessor.count);
  switch(indicesAccessor.componentType) {
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
      auto _buf = (const uint32_t *)buf;
      for(size_t index = 0; index < indicesAccessor.count; index++)
        indices.push_back(_buf[index]);
      break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
      auto _buf = (const uint16_t *)buf;
      for(size_t index = 0; index < indicesAccessor.count; index++)
        indices.push_back(_buf[index]);
      break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
      auto _buf = (const uint8_t *)buf;
      for(size_t index = 0; index < indicesAccessor.count; index++)
        indices.push_back(_buf[index]);
      break;
    }
    default:
      error("Index component type", indicesAccessor.componentType, "not supported!");
  }
}

auto GLTFLoader::loadAnimations(const tinygltf::Model &model) -> void {

  animations.reserve(model.animations.size());
  for(const auto &animation: model.animations) {
    Animation _animation{scene};
    _animation.name = animation.name;
    for(auto &sampler: animation.samplers) {
      AnimationSampler _sampler;
      if(sampler.interpolation == "LINEAR")
        _sampler.interpolation = InterpolationType::Linear;
      else if(sampler.interpolation == "STEP")
        _sampler.interpolation = InterpolationType::Step;
      else if(sampler.interpolation == "CUBICSPLINE")
        _sampler.interpolation = InterpolationType::CubicSpline;
      else
        error("Not supported");
      {
        const auto &accessor = model.accessors[sampler.input];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto &buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        auto buf = reinterpret_cast<const float *>(
          &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for(size_t index = 0; index < accessor.count; index++)
          _sampler.keyTimings.push_back(buf[index]);
      }
      {
        const auto &accessor = model.accessors[sampler.output];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto &buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        auto dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
        switch(accessor.type) {
          case TINYGLTF_TYPE_VEC3: {
            auto buf = reinterpret_cast<const glm::vec3 *>(dataPtr);
            for(size_t index = 0; index < accessor.count; index++) {
              _sampler.keyFrames.emplace_back(buf[index], 0.0f);
            }
            break;
          }
          case TINYGLTF_TYPE_VEC4: {
            auto buf = reinterpret_cast<const glm::vec4 *>(dataPtr);
            for(size_t index = 0; index < accessor.count; index++) {
              _sampler.keyFrames.push_back(buf[index]);
            }
            break;
          }
          default: {
            error("Not supported Animation Type");
            break;
          }
        }
      }
      _animation.samplers.push_back(_sampler);
    }

    for(auto &channel: animation.channels) {
      AnimationChannel _channel;
      if(channel.target_path == "translation") _channel.path = PathType::Translation;
      else if(channel.target_path == "rotation")
        _channel.path = PathType::Rotation;
      else if(channel.target_path == "scale")
        _channel.path = PathType::Scale;
      else
        error("Not supported");
      _channel.samplerIdx = channel.sampler;
      _channel.node = _nodes[channel.target_node];
      _animation.channels.push_back(_channel);
    }

    animations.push_back(_animation);
  }
}
}