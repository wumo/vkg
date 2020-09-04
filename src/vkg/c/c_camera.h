#ifndef VKG_C_CAMERA_H
#define VKG_C_CAMERA_H
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CCamera;
typedef struct CCamera CCamera;

void CameraGetLocation(CCamera *camera, cvec3 *location, uint32_t offset_float);
void CameraSetLocation(CCamera *camera, cvec3 *location, uint32_t offset_float);
void CameraGetDirection(CCamera *camera, cvec3 *direction, uint32_t offset_float);
void CameraSetDirection(CCamera *camera, cvec3 *direction, uint32_t offset_float);
void CameraGetWorldUp(CCamera *camera, cvec3 *worldUp, uint32_t offset_float);
void CameraSetWorldUp(CCamera *camera, cvec3 *worldUp, uint32_t offset_float);
float CameraGetZNear(CCamera *camera);
void CameraSetZNear(CCamera *camera, float zNear);
float CameraGetZFar(CCamera *camera);
void CameraSetZFar(CCamera *camera, float zFar);
float CameraGetFov(CCamera *camera);
uint32_t CameraGetWidth(CCamera *camera);
uint32_t CameraGetHeight(CCamera *camera);
void CameraGetView(CCamera *camera, cmat4 *view, uint32_t offset_float);
void CameraGetProj(CCamera *camera, cmat4 *proj, uint32_t offset_float);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_CAMERA_H
