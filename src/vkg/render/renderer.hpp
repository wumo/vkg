#pragma once
#include "vkg/base/base.hpp"
#include "scene.hpp"
#include <map>

namespace vkg {
class Renderer: public Base {
  friend class RendererSetupPass;

public:
  Renderer(WindowConfig windowConfig, FeatureConfig featureConfig);

  auto addScene(SceneConfig sceneConfig = {}, const std::string &name = "Scene")
    -> Scene &;

protected:
  auto onInit() -> void override;

protected:
  void onFrame(uint32_t imageIndex, float elapsed) override;

private:
  std::map<std::string, std::unique_ptr<Scene>> scenes;

  std::unique_ptr<FrameGraph> frameGraph;
};
}
