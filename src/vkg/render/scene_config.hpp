#pragma once
#include <cstdint>

namespace vkg {
struct SceneConfig {
  /** renderArea */
  int32_t offsetX{0}, offsetY{0};
  uint32_t extentW{0}, extentH{0};
  uint32_t layer{0};

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
}