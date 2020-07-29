#pragma once
#include "vkg/base/base.hpp"
#include "scene.hpp"
#include <map>

namespace vkg {
class Renderer: public Base {

public:
  Renderer(WindowConfig windowConfig, FeatureConfig featureConfig);

  auto addScene(SceneConfig sceneConfig = {}, std::string name = "DefaultScene")
    -> Scene &;

protected:
  auto resize() -> void override;
  auto onInit() -> void override;
  void onFrame(uint32_t imageIndex, float elapsed) override;

private:
  std::map<std::string_view, std::unique_ptr<Scene>> scenes;

  std::unique_ptr<FrameGraph> frameGraph;
};
}
