#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;

auto main() -> int {
  WindowConfig windowConfig{};
  FeatureConfig featureConfig{.numFrames = 2, .rayTrace = true};
  Renderer app{windowConfig, featureConfig};
  SceneConfig sceneConfig{
    .maxNumTransforms = 100'0000,
    .maxNumPrimitives = 100'0000,
    .maxNumMeshInstances = 100'0000};
  auto &scene = app.addScene(sceneConfig);

  auto &sky = scene.atmosphere();
  sky.enable(true);
  sky.setSunIntensity(2);
  sky.setSunDirection({0, -1, -1});

  scene.shadowmap().enable(true);
  scene.shadowmap().setNumCascades(4);
  scene.shadowmap().setZFar(1e3);

  {
    auto yellowMat = scene.newMaterial();
    scene.material(yellowMat).setColorFactor({Yellow, 1.f});
    auto redMat = scene.newMaterial();
    scene.material(redMat).setColorFactor({Red, 1.f});
    auto greenMat = scene.newMaterial();
    scene.material(greenMat).setColorFactor({Green, 1.f});
    auto blueMat = scene.newMaterial();
    scene.material(blueMat).setColorFactor({Blue, 1.f});

    auto primitives = scene.newPrimitives(
      PrimitiveBuilder().axis({}, 10.f, 0.1f, 0.5f, 50).newPrimitive());

    auto originMesh = scene.newMesh(primitives[0], yellowMat);
    auto xMesh = scene.newMesh(primitives[1], redMat);
    auto yMesh = scene.newMesh(primitives[2], greenMat);
    auto zMesh = scene.newMesh(primitives[3], blueMat);
    auto axisNode = scene.newNode();
    scene.node(axisNode).addMeshes({originMesh, xMesh, yMesh, zMesh});
    auto axisModel = scene.newModel({axisNode});

    scene.newModelInstance(axisModel);
  }

  {
    // brdf
    auto primitive =
      scene.newPrimitives(PrimitiveBuilder().sphere({}, 2.f).newPrimitive())[0];
    auto &refractiveMat = scene.material(scene.newMaterial(MaterialType::eRefractive));
    refractiveMat.setColorFactor({White, 1.f}).setPbrFactor({0, 0.3, 0.4, 0});
    auto mesh2 = scene.newMesh(primitive, refractiveMat.id());
    auto node2 = scene.newNode(Transform{{0, 0, 0}});
    scene.node(node2).addMeshes({mesh2});
    auto sphere = scene.newModel({node2});

    for(int i = 0; i <= 10; ++i) {
      float roughness = i * 0.1f;
      for(int j = 0; j <= 10; ++j) {
        float metallic = j * 0.1f;
        auto &refractiveMat =
          scene.material(scene.newMaterial(MaterialType::eRefractive));
        refractiveMat.setColorFactor({White, 1.f})
          .setPbrFactor({0, roughness, metallic, 0});
        auto ins = scene.modelInstance(scene.newModelInstance(
          sphere, Transform{glm::vec3{-20 + 5.f * i, -20 + 5.f * j, 0.f}}));
        ins.setCustomMaterial(refractiveMat.id());
      }
    }
  }

  auto &input = app.window().input();
  auto &camera = scene.camera();
  camera.setLocation({0.307910, 7.145674, 15.825867});
  camera.setZFar(1e9);
  PanningCamera panningCamera{camera};
  bool pressed{false};
  auto &fpsMeter = app.fpsMeter();
  app.loop([&](uint32_t frameIdx, double elapsed) {
    static auto lastChange = std::chrono::high_resolution_clock::now();

    app.window().setWindowTitle(toString(
      "FPS: ", fpsMeter.fps(), " Frame Time: ", std::round(fpsMeter.frameTime()), " ms"));

    panningCamera.update(input);
    //    auto loc = camera.location();
    //    println(
    //      "camera loc:", glm::to_string(loc),
    //      " camera dir:", glm::to_string(camera.direction()));
  });
  return 0;
}