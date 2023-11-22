#include "c_light.h"
#include "vkg/render/scene.hpp"
using namespace vkg;

uint32_t LightGetCount(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    return light.count();
}
void LightGetColor(CScene *scene, uint32_t id, cvec3 *color, uint32_t offset_float) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    auto color_ = light.color();
    memcpy((float *)color + offset_float, &color_, sizeof(color_));
}
void LightSetColor(CScene *scene, uint32_t id, cvec3 *color, uint32_t offset_float) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    light.setColor(*(glm::vec3 *)((float *)color + offset_float));
}
void LightGetLocation(CScene *scene, uint32_t id, cvec3 *location, uint32_t offset_float) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    auto location_ = light.location();
    memcpy((float *)location + offset_float, &location_, sizeof(location_));
}
void LightSetLocation(CScene *scene, uint32_t id, cvec3 *location, uint32_t offset_float) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    light.setLocation(*(glm::vec3 *)((float *)location + offset_float));
}
float LightGetIntensity(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    return light.intensity();
}
void LightSetIntensity(CScene *scene, uint32_t id, float intensity) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    light.setIntensity(intensity);
}
float LightGetRange(CScene *scene, uint32_t id) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    return light.range();
}
void LightSetRange(CScene *scene, uint32_t id, float range) {
    auto *scene_ = reinterpret_cast<Scene *>(scene);
    auto &light = scene_->light(id);
    light.setRange(range);
}
