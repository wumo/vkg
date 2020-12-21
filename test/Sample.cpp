#include "vkg/render/renderer.hpp"
#include "vkg/util/colors.hpp"
#include "vkg/render/util/panning_camera.hpp"
using namespace vkg;
auto main() -> int {
  // window and renderer setting
  WindowConfig windowConfig{.title = "Sample", .width = 1080, .height = 720};
  FeatureConfig featureConfig{.numFrames = 1, .rayTrace = true};
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
  sky.setSunIntensity(2);
  sky.setSunDirection({-1, -0.1, 0});

  auto yellowMat = scene.newMaterial();
  scene.material(yellowMat).setColorFactor({Yellow, 1.f});
  auto redMat = scene.newMaterial();
  scene.material(redMat).setColorFactor({Red, 1.f});
  auto greenMat = scene.newMaterial();
  scene.material(greenMat).setColorFactor({Green, 1.f});
  auto blueMat = scene.newMaterial();
  scene.material(blueMat).setColorFactor({Blue, 1.f});
  auto colorTex = scene.newTexture(
    "./assets/glTF-models/2.0/TextureCoordinateTest/glTF/TextureCoordinateTemplate.png");
  auto texMat = scene.newMaterial(MaterialType::eBRDF);
  scene.material(texMat).setColorTex(colorTex).setPbrFactor({0, 0.3, 0.4, 0});

  {
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

  float total=0;
  //render loop
  app.loop([&](uint32_t frameIdx, double elapsedMs) {
    // update camera from input
    panningCamera.update(input);

    total+=elapsedMs;

    // apply transform per frame
    auto &ins = scene.modelInstance(sphere);
    if(total>2000){
      ins.setVisible(false);
    }

    auto t = ins.transform();
    t.translation.x -= elapsedMs * 0.001;
    ins.setTransform(t);
  });

  return 0;
}