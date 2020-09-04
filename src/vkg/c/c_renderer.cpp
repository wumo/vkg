#include "c_renderer.h"
#include "vkg/render/renderer.hpp"
using namespace vkg;
CRenderer *NewRenderer(CWindowConfig windowConfig, CFeatureConfig featureConfig) {
  WindowConfig windowConfig_{
    .title = std::string(windowConfig.title),
    .width = windowConfig.width,
    .height = windowConfig.height,
  };
  FeatureConfig featureConfig_{
    .fullscreen = featureConfig.fullscreen,
    .vsync = featureConfig.vsync,
    .numFrames = featureConfig.numFrames,
    .rayTrace = featureConfig.rayTrace,
  };
  return reinterpret_cast<CRenderer *>(new Renderer{windowConfig_, featureConfig_});
}
void DeleteRenderer(CRenderer *renderer) {
  delete reinterpret_cast<Renderer *>(renderer);
}
void RendererLoopFuncPtr(CRenderer *renderer, CUpdater updater, void *data) {
  auto *renderer_ = reinterpret_cast<Renderer *>(renderer);
  renderer_->loop(updater, data);
}
void RendererLoopUpdater(CRenderer *renderer, CCallFrameUpdater *updater) {
  auto *renderer_ = reinterpret_cast<Renderer *>(renderer);
  auto *updater_ = reinterpret_cast<CallFrameUpdater *>(updater);
  renderer_->loop(*updater_);
}

CWindow *RendererGetWindow(CRenderer *renderer) {
  auto *renderer_ = reinterpret_cast<Renderer *>(renderer);
  return reinterpret_cast<CWindow *>(&renderer_->window());
}

CScene *RendererAddScene(
  CRenderer *renderer, CSceneConfig sceneConfig, char *nameBuf, uint32_t size) {
  auto *renderer_ = reinterpret_cast<Renderer *>(renderer);
  auto sceneConfig_ = *(SceneConfig *)&sceneConfig;
  return reinterpret_cast<CScene *>(
    &renderer_->addScene(sceneConfig_, std::string{nameBuf, size}));
}
CFPSMeter *RenderGetFPSMeter(CRenderer *renderer) {
  auto *renderer_ = reinterpret_cast<Renderer *>(renderer);
  return reinterpret_cast<CFPSMeter *>(&renderer_->fpsMeter());
}
