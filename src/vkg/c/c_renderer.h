#ifndef VKG_C_RENDERER_H
#define VKG_C_RENDERER_H
#include "c_scene.h"
#include "c_window.h"
#include <cstdint>
#ifdef __cplusplus
extern "C" {
#else
  #include <stdbool.h>
#endif

typedef struct {
  const char *title;
  uint32_t width, height;
} CWindowConfig;

typedef struct {
  bool fullscreen;
  bool vsync;
  uint32_t numFrames;
  bool rayTrace;
} CFeatureConfig;

struct CRenderer;
typedef struct CRenderer CRenderer;

CRenderer *NewRenderer(CWindowConfig windowConfig, CFeatureConfig featureConfig);
void DeleteRenderer(CRenderer *renderer);

typedef void (*CUpdater)(uint32_t frameIdx, double elapsedMs, void *data);
void RendererLoopFuncPtr(CRenderer *renderer, CUpdater updater, void *data);

struct CCallFrameUpdater;
typedef struct CCallFrameUpdater CCallFrameUpdater;
void RendererLoopUpdater(CRenderer *renderer, CCallFrameUpdater *updater);

CWindow *RendererGetWindow(CRenderer *renderer);

CScene *RendererAddScene(
  CRenderer *renderer, CSceneConfig sceneConfig, const char *nameBuf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif //VKG_C_RENDERER_H
