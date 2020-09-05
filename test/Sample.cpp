#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;
auto main() -> int {
  // window and renderer setting
  WindowConfig windowConfig{.title = "Sample", .width = 1080, .height = 720};
  FeatureConfig featureConfig{.numFrames = 2, .rayTrace = true};
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
  sky.setSunIntensity(10);
  sky.setSunDirection({-1, -0.1, 0});

  // primitive
  auto primitives =
    scene.newPrimitives(PrimitiveBuilder().sphere({}, 1.f).newPrimitive());
  // material
  auto &mat = scene.material(scene.newMaterial(MaterialType::eBRDF));
  mat.setColorFactor({Green, 1.f}).setPbrFactor({0, 0.3, 0.4, 0});
  // primitive + material => mesh
  auto mesh = scene.newMesh(primitives[0], mat.id());
  // mesh => node
  auto node = scene.newNode(Transform{});
  scene.node(node).addMeshes({mesh});
  // node => model
  auto model = scene.newModel({node});
  // model => instance
  auto sphere = scene.newModelInstance(model);

  // scene camera setting
  auto &camera = scene.camera();
  camera.setLocation({-5.610391, 0.049703, 16.386591});
  camera.setDirection({5.610391, -0.049703, -16.386591});

  // using builtin panning camera to change view
  PanningCamera panningCamera{camera};
  // capture input
  auto &input = app.window().input();

  //render loop
  app.loop([&](uint32_t frameIdx, double elapsedMs) {
    // update camera from input
    panningCamera.update(input);

    // apply transform per frame
    auto &ins = scene.modelInstance(sphere);
    auto t = ins.transform();
    t.translation.x -= elapsedMs * 0.001;
    ins.setTransform(t);
  });

  return 0;
}