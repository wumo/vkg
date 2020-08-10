#pragma once
#include "vkg/base/base.hpp"
#include "scene.hpp"
#include <map>

namespace vkg {
struct RendererPassIn {};
struct RendererPassOut {
  FrameGraphResource<vk::Extent2D> swapchainExtent;
  FrameGraphResource<vk::Format> swapchainFormat;
  FrameGraphResource<uint64_t> swapchainVersion;
};
class Renderer: public Base, public Pass<RendererPassIn, RendererPassOut> {
  friend class Scene;

public:
  Renderer(WindowConfig windowConfig, FeatureConfig featureConfig);

  auto addScene(SceneConfig sceneConfig = {}, const std::string &name = "Scene")
    -> Scene &;

protected:
  auto onInit() -> void override;

public:
  auto setup(PassBuilder &builder, const RendererPassIn &inputs)
    -> RendererPassOut override;
  void compile(Resources &resources) override;

protected:
  void onFrame(uint32_t imageIndex, float elapsed) override;

private:
  std::map<std::string, std::unique_ptr<Scene>> scenes;

  std::unique_ptr<FrameGraph> frameGraph;
  RendererPassOut passOut;
};
}
