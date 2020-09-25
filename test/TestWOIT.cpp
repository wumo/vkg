#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;
auto main() -> int {
  // window and renderer setting
  WindowConfig windowConfig{.title = "Sample", .width = 1960, .height = 1080};
  FeatureConfig featureConfig{.numFrames = 2, .rayTrace = false};
  Renderer app{windowConfig, featureConfig};

  // scene setting
  SceneConfig sceneConfig{
    .maxNumTransforms = 100'0000,
    .maxNumPrimitives = 100'0000,
    .maxNumMeshInstances = 100'0000,
  };
  auto &scene = app.addScene(sceneConfig);

  // atmosphere setting
  auto &sky = scene.atmosphere();
  sky.enable(true);
  sky.setSunIntensity(1);
  sky.setSunDirection({-1, -0.1, 0});

  { // transparent
    auto primitives = scene.newPrimitives(PrimitiveBuilder()
                                            .rectangle({}, {0, 0, 5000}, {5000, 0, 0})
                                            .newPrimitive()
                                            .box({}, {0, 0, 0.5}, {0.5, 0, 0}, 0.5)
                                            .newPrimitive());
    {
      auto mat = scene.newMaterial(MaterialType::eNone);
      scene.material(mat).setColorFactor({White, 1.f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[0], mat)});
      scene.newModelInstance(scene.newModel({node}), Transform{{5000, -1, 5000}});
    }
    {
      auto mat = scene.newMaterial(MaterialType::eTransparent);
      scene.material(mat).setColorFactor({Yellow, 0.5f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[1], mat)});
      scene.newModelInstance(
        scene.newModel({node}), Transform{{5000, 1000, 5000}, {1000, 1000, 1000}});
    }
  }

  // scene camera setting
  auto &camera = scene.camera();
  camera.setLocation({-4275.704102, 6248.930664, 4847.410156});
  camera.setDirection({6851.990234, -6248.930176, -0.000977});
  camera.setZFar(1e9);
  // using builtin panning camera to change view
  PanningCamera panningCamera{camera};
  // capture input
  auto &input = app.window().input();

  //render loop
  app.loop([&](uint32_t frameIdx, double elapsedMs) {
    // update camera from input
    panningCamera.update(input);
    auto loc = camera.location();
    println(
      "camera loc:", glm::to_string(loc),
      " camera dir:", glm::to_string(camera.direction()));
  });

  return 0;
}