#ifndef VKG_C_SCENE_H
#define VKG_C_SCENE_H

#include "c_primitive.h"
#include "c_primitive_builder.h"
#include "c_material.h"
#include "c_mesh.h"
#include "c_node.h"
#include "c_model.h"
#include "c_model_instance.h"
#include "c_light.h"
#include "c_camera.h"
#include "c_atmosphere.h"
#include "c_shadowmap.h"
#include "vkg/base/resource/texture_formats.h"

#ifdef __cplusplus
extern "C" {
#else
    #include <stdbool.h>
#endif

struct CScene;
typedef struct CScene CScene;

typedef struct {
    /** renderArea */
    int32_t offsetX, offsetY;
    uint32_t extentW, extentH;
    uint32_t layer;

    /**max number of vertices and indices*/
    uint32_t maxNumVertices, maxNumIndices;
    /**max number of node and instance transforms*/
    uint32_t maxNumTransforms;
    /**max number of materials*/
    uint32_t maxNumMaterials;
    /**max number of mesh instances*/
    uint32_t maxNumPrimitives;
    uint32_t maxNumMeshInstances;
    /**max number of texture including 2d and cube map.*/
    uint32_t maxNumTexture;
    /**max number of lights*/
    uint32_t maxNumLights;
} CSceneConfig;

uint32_t SceneNewPrimitive(
    CScene *scene, cvec3 *positions, uint32_t position_offset_float, uint32_t numPositions, cvec3 *normals,
    uint32_t normal_offset_float, uint32_t numNormals, cvec2 *uvs, uint32_t uv_offset_float, uint32_t numUVs,
    uint32_t *indices, uint32_t numIndices, caabb *aabb, CPrimitiveTopology topology, bool perFrame);
void SceneNewPrimitives(CScene *scene, CPrimitiveBuilder *builder, bool perFrame, uint32_t *ptrs);

uint32_t SceneNewMaterial(CScene *scene, CMaterialType type, bool perFrame);
uint32_t SceneNewTexture(CScene *scene, char *pathBuf, uint32_t pathSize, bool mipmap);
uint32_t SceneNewTextureFromBytes(
    CScene *scene, const char *bytes, uint32_t numBytes, uint32_t width, uint32_t height, TextureFormat format,
    bool mipmap);
uint32_t SceneNewTextureFromMemory(CScene *scene, const char *bytes, uint32_t numBytes, bool mipmap);
uint32_t SceneNewMesh(CScene *scene, uint32_t primitive, uint32_t material);
uint32_t SceneNewNode(CScene *scene, ctransform *transform);
uint32_t SceneNewModel(CScene *scene, uint32_t *nodes, uint32_t numNodes);
uint32_t SceneLoadModel(CScene *scene, char *pathBuf, uint32_t pathSize, CMaterialType type);
uint32_t SceneLoadModelFromBytes(CScene *scene, const char *bytes, uint32_t numBytes, CMaterialType type);
uint32_t SceneNewModelInstance(CScene *scene, uint32_t model, ctransform *transform, bool perFrame);
uint32_t SceneNewLight(CScene *scene, bool perFrame);

CCamera *SceneGetCamera(CScene *scene);
CAtmosphereSetting *SceneGetAtmosphere(CScene *scene);
CShadowMapSetting *SceneGetShadowmap(CScene *scene);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_SCENE_H
