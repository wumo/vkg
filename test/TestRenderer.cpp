#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;

auto main() -> int {
  WindowConfig windowConfig{};
  FeatureConfig featureConfig{};
  Renderer app{windowConfig, featureConfig};

  auto &scene = app.addScene();

  auto lightId = scene.newLight();
  auto &light = scene.light(lightId);
  light.setColor(White);
  glm::vec3 loc{100, 200, 200};
  loc = glm::angleAxis(glm::radians(-60.f), glm::vec3{0, 1, 0}) * loc;
  light.setLocation(loc);
  light.setIntensity(1);

  auto yellowMat = scene.newMaterial();
  scene.material(yellowMat).setColorFactor({Yellow, 1.f});
  auto redMat = scene.newMaterial();
  scene.material(redMat).setColorFactor({Red, 1.f});
  auto greenMat = scene.newMaterial();
  scene.material(greenMat).setColorFactor({Green, 1.f});
  auto blueMat = scene.newMaterial();
  scene.material(blueMat).setColorFactor({Blue, 1.f});
  auto colorTex = scene.newTexture("./assets/TextureCoordinateTemplate.png");
  auto texMat = scene.newMaterial(MaterialType::eBRDF);
  scene.material(texMat).setColorTex(colorTex).setPbrFactor({0, 0.3, 0.4, 0});

  auto primitives =
    scene.newPrimitives(PrimitiveBuilder().axis({}, 10.f, 0.1f, 0.5f, 50).newPrimitive());

  auto originMesh = scene.newMesh(primitives[0], yellowMat);
  auto xMesh = scene.newMesh(primitives[1], redMat);
  auto yMesh = scene.newMesh(primitives[2], greenMat);
  auto zMesh = scene.newMesh(primitives[3], blueMat);
  auto axisNode = scene.newNode();
  scene.node(axisNode).addMeshes({originMesh, xMesh, yMesh, zMesh});
  auto axisModel = scene.newModel({axisNode});

  for(int i = 0; i < 1; ++i) {
    scene.newModelInstance(axisModel);
  }

  { // texture
    auto primitive =
      scene.newPrimitives(PrimitiveBuilder()
                            .rectangle({}, {5.f, 0.f, -5.f}, {0.f, 10.f, 0.f})
                            .newPrimitive())[0];
    auto mesh = scene.newMesh(primitive, texMat);
    auto node = scene.newNode();
    scene.node(node).addMeshes({mesh});
    auto model = scene.newModel({node});
    scene.newModelInstance(model);
  }

  auto &input = app.window().input();
  auto &camera = scene.camera();
  //  camera.setLocation({20.f, 20.f, 20.f});
  camera.setLocation({22.759, 8.61548, 26.1782});
  camera.setZFar(1e3);
  PanningCamera panningCamera{camera};
  bool pressed{false};
  app.loop([&](double elapsed) { panningCamera.update(input); });
  return 0;
}