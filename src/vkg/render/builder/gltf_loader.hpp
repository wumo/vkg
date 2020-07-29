#pragma once
#include <tiny_gltf.h>
#include "vkg/base/vk_headers.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/model/animation.hpp"
#include "vkg/render/model/aabb.hpp"
#include "vkg/render/model/material.hpp"

namespace vkg {
class Scene;
class GLTFLoader {
public:
  explicit GLTFLoader(Scene &scene, MaterialType materialType);

  auto load(const std::string &file) -> uint32_t;

private:
  void loadTextureSamplers(const tinygltf::Model &model);
  void loadTextures(const tinygltf::Model &model);
  void loadMaterials(const tinygltf::Model &model);
  auto loadNode(int thisID, const tinygltf::Model &model) -> uint32_t;
  auto loadPrimitive(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
    -> uint32_t;
  AABB loadVertices(const tinygltf::Model &model, const tinygltf::Primitive &primitive);
  auto loadIndices(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
    -> void;
  auto loadAnimations(const tinygltf::Model &model) -> void;

  Scene &scene;
  MaterialType defaultMatType;
  std::vector<uint32_t> indices;
  std::vector<Vertex::Position> positions;
  std::vector<Vertex::Normal> normals;
  std::vector<Vertex::UV> uvs;
  std::vector<Vertex::Joint> joint0s;
  std::vector<Vertex::Weight> weight0s;
  std::vector<uint32_t> _nodes;
  std::vector<Animation> animations;

  std::vector<vk::SamplerCreateInfo> samplerDefs;
  std::vector<uint32_t> textures;
  std::vector<uint32_t> materials;
};
}
