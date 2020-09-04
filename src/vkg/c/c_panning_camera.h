#ifndef VKG_C_PANNING_CAMERA_H
#define VKG_C_PANNING_CAMERA_H
#include "vkg/base/window/input.h"
#include "c_camera.h"
#include <cstdint>
#include "c_vec.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CPanningCamera;
typedef struct CPanningCamera CPanningCamera;

CPanningCamera *NewPanningCamera(CCamera *camera);
void DeletePanningCamera(CPanningCamera *camera);
void PanningCameraUpdate(CPanningCamera *camera, Input *input);

#ifdef __cplusplus
}
#endif
#endif //VKG_C_PANNING_CAMERA_H
