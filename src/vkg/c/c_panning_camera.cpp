#include "c_panning_camera.h"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;
CPanningCamera *NewPanningCamera(CCamera *camera) {
    auto *camera_ = reinterpret_cast<Camera *>(camera);
    return reinterpret_cast<CPanningCamera *>(new PanningCamera(*camera_));
}
void DeletePanningCamera(CPanningCamera *camera) {
    auto *camera_ = reinterpret_cast<PanningCamera *>(camera);
    delete camera_;
}
void PanningCameraUpdate(CPanningCamera *camera, Input *input) {
    auto *camera_ = reinterpret_cast<PanningCamera *>(camera);
    camera_->update(*input);
}
