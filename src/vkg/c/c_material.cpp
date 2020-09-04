#include "c_material.h"
#include "vkg/render/scene.hpp"

using namespace vkg;
uint32_t MaterialGetCount(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.count();
}
uint32_t MaterialGetColorTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.colorTex();
}
void MaterialSetColorTex(CScene *scene, uint32_t id, uint32_t colorTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setColorTex(colorTex);
}
uint32_t MaterialGetPbrTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.pbrTex();
}
void MaterialSetPbrTex(CScene *scene, uint32_t id, uint32_t pbrTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setPbrTex(pbrTex);
}
uint32_t MaterialGetNormalTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.normalTex();
}
void MaterialSetNormalTex(CScene *scene, uint32_t id, uint32_t normalTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setNormalTex(normalTex);
}
uint32_t MaterialGetOcclusionTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.occlusionTex();
}
void MaterialSetOcclusionTex(CScene *scene, uint32_t id, uint32_t occlusionTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setOcclusionTex(occlusionTex);
}
uint32_t MaterialGetEmissiveTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.emissiveTex();
}
void MaterialSetEmissiveTex(CScene *scene, uint32_t id, uint32_t emissiveTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setEmissiveTex(emissiveTex);
}
void MaterialGetColorFactor(
  CScene *scene, uint32_t id, cvec4 *colorFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  auto colorFactor_ = material.colorFactor();
  memcpy((float *)colorFactor + offset_float, &colorFactor_, sizeof(colorFactor_));
}
void MaterialSetColorFactor(
  CScene *scene, uint32_t id, cvec4 *colorFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setColorFactor(*(glm::vec4 *)((float *)colorFactor + offset_float));
}
void MaterialGetPbrFactor(
  CScene *scene, uint32_t id, cvec4 *pbrFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  auto pbrFactor_ = material.pbrFactor();
  memcpy((float *)pbrFactor + offset_float, &pbrFactor_, sizeof(pbrFactor_));
}
void MaterialSetPbrFactor(
  CScene *scene, uint32_t id, cvec4 *pbrFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setPbrFactor(*(glm::vec4 *)((float *)pbrFactor + offset_float));
}
float MaterialGetOcclusionStrength(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.occlusionStrength();
}
void MaterialSetOcclusionStrength(CScene *scene, uint32_t id, float occlusionStrength) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setOcclusionStrength(occlusionStrength);
}
float MaterialGetAlphaCutoff(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.alphaCutoff();
}
void MaterialSetAlphaCutoff(CScene *scene, uint32_t id, float alphaCutoff) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setAlphaCutoff(alphaCutoff);
}
void MaterialGetEmissiveFactor(
  CScene *scene, uint32_t id, cvec4 *emissiveFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  auto emissiveFactor_ = material.emissiveFactor();
  memcpy(
    (float *)emissiveFactor + offset_float, &emissiveFactor_, sizeof(emissiveFactor_));
}
void MaterialSetEmissiveFactor(
  CScene *scene, uint32_t id, cvec4 *emissiveFactor, uint32_t offset_float) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setEmissiveFactor(*(glm::vec4 *)((float *)emissiveFactor + offset_float));
}
uint32_t MaterialGetHeightTex(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return material.heightTex();
}
void MaterialSetHeightTex(CScene *scene, uint32_t id, uint32_t heightTex) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  material.setHeightTex(heightTex);
}
CMaterialType MaterialGetType(CScene *scene, uint32_t id) {
  auto *scene_ = reinterpret_cast<Scene *>(scene);
  auto &material = scene_->material(id);
  return static_cast<CMaterialType>(material.type());
}
