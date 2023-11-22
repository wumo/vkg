#include "c_camera.h"
#include "vkg/render/model/camera.hpp"
using namespace vkg;

using namespace vkg;
void CameraGetLocation(CCamera *camera, cvec3 *location, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    auto location_ = cam->location();
    memcpy((float *)location + offset_float, &location_, sizeof(location_));
}
void CameraSetLocation(CCamera *camera, cvec3 *location, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    cam->setLocation(*(glm::vec3 *)((float *)location + offset_float));
}
void CameraGetDirection(CCamera *camera, cvec3 *direction, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    auto direction_ = cam->direction();
    memcpy((float *)direction + offset_float, &direction_, sizeof(direction_));
}
void CameraSetDirection(CCamera *camera, cvec3 *direction, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    cam->setDirection(*(glm::vec3 *)((float *)direction + offset_float));
}
void CameraGetWorldUp(CCamera *camera, cvec3 *worldUp, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    auto worldUp_ = cam->worldUp();
    memcpy((float *)worldUp + offset_float, &worldUp_, sizeof(worldUp_));
}
void CameraSetWorldUp(CCamera *camera, cvec3 *worldUp, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    cam->setWorldUp(*(glm::vec3 *)((float *)worldUp + offset_float));
}
float CameraGetZNear(CCamera *camera) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    return cam->zNear();
}
void CameraSetZNear(CCamera *camera, float zNear) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    cam->setZNear(zNear);
}
float CameraGetZFar(CCamera *camera) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    return cam->zFar();
}
void CameraSetZFar(CCamera *camera, float zFar) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    cam->setZFar(zFar);
}
float CameraGetFov(CCamera *camera) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    return cam->fov();
}
uint32_t CameraGetWidth(CCamera *camera) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    return cam->width();
}
uint32_t CameraGetHeight(CCamera *camera) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    return cam->height();
}
void CameraGetView(CCamera *camera, cmat4 *view, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    auto view_ = cam->view();
    memcpy((float *)view + offset_float, &view_, sizeof(view_));
}
void CameraGetProj(CCamera *camera, cmat4 *proj, uint32_t offset_float) {
    auto *cam = reinterpret_cast<Camera *>(camera);
    auto proj_ = cam->proj();
    memcpy((float *)proj + offset_float, &proj_, sizeof(proj_));
}
