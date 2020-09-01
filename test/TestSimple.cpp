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

  auto kPi = glm::pi<float>();
  float seasonAngle = kPi / 4;
  float sunAngle = 0;
  float angularVelocity = kPi / 20;
  auto sunDirection = [&](float dt) {
    sunAngle += angularVelocity * dt;
    if(sunAngle > 2 * kPi) sunAngle = 0;

    return -glm::vec3{
      glm::cos(sunAngle), glm::abs(glm::sin(sunAngle) * glm::sin(seasonAngle)),
      -glm::sin(sunAngle) * glm::cos(seasonAngle)};
  };
  auto sunDir = sunDirection(1);
  auto &sky = scene.atmosphere();
  sky.enable(false);
  sky.setSunIntensity(10);
  sky.setSunDirection(sunDir);

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

//  auto primitives =
//    scene.newPrimitives(PrimitiveBuilder().axis({}, 10.f, 0.1f, 0.5f, 50).newPrimitive());
//
//  auto originMesh = scene.newMesh(primitives[0], yellowMat);
//  auto xMesh = scene.newMesh(primitives[1], redMat);
//  auto yMesh = scene.newMesh(primitives[2], greenMat);
//  auto zMesh = scene.newMesh(primitives[3], blueMat);
//  auto axisNode = scene.newNode();
//  scene.node(axisNode).addMeshes({originMesh, xMesh, yMesh, zMesh});
//  auto axisModel = scene.newModel({axisNode});
//
//  scene.newModelInstance(axisModel);

//  { // texture
//    auto primitive =
//      scene.newPrimitives(PrimitiveBuilder()
//                            .rectangle({}, {5.f, 0.f, -5.f}, {0.f, 10.f, 0.f})
//                            .newPrimitive())[0];
//    auto mesh = scene.newMesh(primitive, texMat);
//    auto node = scene.newNode();
//    scene.node(node).addMeshes({mesh});
//    auto model = scene.newModel({node});
//    scene.newModelInstance(model);
//  }

  //  { // brdf
  //    auto primitives =
  //      scene.newPrimitives(PrimitiveBuilder()
  //                            .checkerboard(100, 100, {}, {0, 0, 1}, {1, 0, 0}, 4, 4)
  //                            .newPrimitive()
  //                            .line({0, 0, 4}, {4, 0, 0})
  //                            .newPrimitive(PrimitiveTopology::Lines));
  //    auto &pbrMat = scene.material(scene.newMaterial(MaterialType::eBRDF));
  //    auto gridTex = scene.newTexture("./assets/grid.png");
  //    pbrMat.setColorTex(gridTex).setPbrFactor({0, 1, 0, 0});
  //    auto mesh = scene.newMesh(primitives[0], pbrMat.id());
  //    auto node = scene.newNode(Transform{{1, 0, 1}});
  //    scene.node(node).addMeshes({mesh});
  //    auto model = scene.newModel({node});
  //    scene.newModelInstance(model);
  //
  //    auto node2 = scene.newNode();
  //    scene.node(node2).addMeshes({scene.newMesh(primitives[1], pbrMat.id())});
  //    scene.newModelInstance(scene.newModel({node2}));
  //  }

  {
    // brdf
    auto primitives = scene.newPrimitives(PrimitiveBuilder()
                                            .box({}, {0, 0, 1}, {1, 0, 0}, 1.f)
                                            .newPrimitive()
                                            .sphere({}, 2.f)
                                            .newPrimitive());
    auto &reflectiveMat = scene.material(scene.newMaterial(MaterialType::eReflective));
    reflectiveMat.setColorFactor({Green, 1.f}).setPbrFactor({0, 0.3, 0.4, 0});
    auto mesh = scene.newMesh(primitives[0], reflectiveMat.id());
    auto node = scene.newNode(Transform{{10, 1, 0}});
    scene.node(node).addMeshes({mesh});
    auto model = scene.newModel({node});
    scene.newModelInstance(model);

    auto &refractiveMat = scene.material(scene.newMaterial(MaterialType::eRefractive));
    refractiveMat.setColorFactor({White, 1.f}).setPbrFactor({0, 0.3, 0.4, 0});
    auto mesh2 = scene.newMesh(primitives[1], refractiveMat.id());
    auto node2 = scene.newNode(Transform{{0, 2, 10}});
    scene.node(node2).addMeshes({mesh2});
    auto model2 = scene.newModel({node2});
    scene.newModelInstance(model2);
  }

  //  uint32_t animModel{};
  //  std::vector<uint32_t> insts;
  //  {
  //    std::string name = "DamagedHelmet";
  //    animModel =
  //      scene.loadModel("assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf");
  //
  //    //    auto primitive =
  //    //      scene.newPrimitive(PrimitiveBuilder()
  //    //                           .box({}, glm::vec3{0, 0, 1}, glm::vec3{1, 0, 0}, 1)
  //    //                           .newPrimitive())[0];
  //    //    auto mesh = scene.newMesh(primitive, redMat);
  //    //    auto node = scene.newNode();
  //    //    scene.node(node).addMeshes({mesh});
  //    //    animModel = scene.newModel({node});
  //
  //    auto &model = scene.model(animModel);
  //    auto aabb = model.aabb();
  //    auto range = aabb.max - aabb.min;
  //    auto scale = 5 / std::max(std::max(range.x, range.y), range.z);
  //    auto center = aabb.center();
  //    auto halfRange = aabb.halfRange();
  //    Transform t{
  //      {-center * scale + glm::vec3{8, scale * range.y / 2.f, 8}}, glm::vec3{scale}};
  //    //  t.translation = -center;
  //    scene.newModelInstance(animModel, t, false);
  //
  //    uint32_t num = 100;
  //    insts.reserve(num * num);
  //    float unit = -5;
  //    for(int a = 0; a < num; ++a) {
  //      for(int b = 0; b < num; ++b) {
  //        t.translation = -center * scale +
  //                        glm::vec3{-10 + unit * a, scale * range.y / 2.f, -10 + unit * b};
  //        insts.push_back(scene.newModelInstance(animModel, t, true));
  //      }
  //    }
  //  }

  auto &input = app.window().input();
  auto &camera = scene.camera();
  //    camera.setLocation({20.f, 0.f, 20.f});
  camera.setLocation({22.759, 8.61548, 26.1782});
  camera.setZFar(1e9);
  PanningCamera panningCamera{camera};
  bool pressed{false};
  app.loop([&](uint32_t frameIdx, double elapsed) {
    static auto lastChange = std::chrono::high_resolution_clock::now();

    panningCamera.update(input);
  });
  return 0;
}