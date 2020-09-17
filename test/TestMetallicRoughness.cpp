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
    std::string name = "MetalRoughSpheres";
    auto modelId = scene.loadModel(
      "assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf",
      MaterialType::eReflective);
    auto &model = scene.model(modelId);
    auto aabb = model.aabb();
    auto range = aabb.max - aabb.min;
    auto scale = 10 / std::max(std::max(range.x, range.y), range.z);
    auto center = aabb.center();
    auto halfRange = aabb.halfRange();
    Transform t{
      {-center * scale + glm::vec3{0, scale * range.y / 2.f, 0}}, glm::vec3{scale}};
    //  t.translation = -center;
    scene.newModelInstance(modelId, t, false);
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