#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
using namespace vkg;

auto main() -> int {
  WindowConfig windowConfig{};
  FeatureConfig featureConfig{};
  Renderer app{windowConfig, featureConfig};

  auto &scene = app.addScene();

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

  app.loop([&](double elapsed) {});
  return 0;
}