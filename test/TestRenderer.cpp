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
  float sunAngle = kPi / 4;
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
  sky.enable(true);
  sky.setSunIntensity(2);
  sky.setSunDirection(sunDir);

  scene.shadowmap().enable(true);
  scene.shadowmap().setNumCascades(4);
  scene.shadowmap().setZFar(1e3);

  //  auto lightId = scene.newLight();
  //  auto &light = scene.light(lightId);
  //  light.setColor(White);
  //  glm::vec3 loc{100, 200, 200};
  //  loc = glm::angleAxis(glm::radians(-60.f), glm::vec3{0, 1, 0}) * loc;
  //  light.setLocation(loc);
  //  light.setIntensity(1);

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

  uint32_t parentNodeId{}, childNodeId{};
  { // node graph
    auto primitive = scene.newPrimitives(
      PrimitiveBuilder().box({}, {0, 7, 0}, {0, 1, 0}, 0.2f, 0.2f).newPrimitive())[0];
    childNodeId = scene.newNode(Transform{
      {0, 7, 0}, {1, 1, 1}, glm::angleAxis(glm::radians(60.f), glm::vec3{1, 0, 0})});
    scene.node(childNodeId).addMeshes({scene.newMesh(primitive, redMat)});

    parentNodeId = scene.newNode(Transform{
      {0, 0, 0}, {1, 1, 1}, glm::angleAxis(glm::radians(30.f), glm::vec3{1, 0, 0})});
    scene.node(parentNodeId).addMeshes({scene.newMesh(primitive, yellowMat)});
    scene.node(parentNodeId).addChildren({childNodeId});
    auto model = scene.newModel({parentNodeId});
    scene.newModelInstance(model);
  }

  { // brdf
    auto primitives =
      scene.newPrimitives(PrimitiveBuilder()
                            .checkerboard(100, 100, {}, {0, 0, 1}, {1, 0, 0}, 4, 4)
                            .newPrimitive()
                            .line({0, 0, 4}, {4, 0, 0})
                            .newPrimitive(PrimitiveTopology::Lines));
    auto &pbrMat = scene.material(scene.newMaterial(MaterialType::eReflective));
    auto gridTex = scene.newTexture("./assets/grid.png");
    pbrMat.setColorTex(gridTex).setPbrFactor({0, 1, 0, 0});
    auto mesh = scene.newMesh(primitives[0], pbrMat.id());
    auto node = scene.newNode(Transform{{0, 0, 0}});
    scene.node(node).addMeshes({mesh});
    auto model = scene.newModel({node});
    scene.newModelInstance(model);

    auto node2 = scene.newNode();
    scene.node(node2).addMeshes({scene.newMesh(primitives[1], pbrMat.id())});
    scene.newModelInstance(scene.newModel({node2}));
  }

  { // lines
    auto primitive = scene.newPrimitives(PrimitiveBuilder()
                                           .line({0, 1, 4}, {4, 1, 0})
                                           .newPrimitive(PrimitiveTopology::Lines))[0];
    auto tRedMat = scene.newMaterial(MaterialType::eNone);
    scene.material(tRedMat).setColorFactor({Green, 0.5f});
    auto node = scene.newNode();
    scene.node(node).addMeshes({scene.newMesh(primitive, tRedMat)});
    scene.newModelInstance(scene.newModel({node}));
  }

  { // transparent
    auto primitives = scene.newPrimitives(PrimitiveBuilder()
                                            .rectangle({}, {-2,0,0}, {0,2,0})
                                            .newPrimitive()
                                            .line({4, 0, 4}, {4, 4, 4})
                                            .newPrimitive(PrimitiveTopology::Lines));
    auto tRedMat = scene.newMaterial(MaterialType::eTransparent);
    {
      scene.material(tRedMat).setColorFactor({Red, 0.5f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[0], tRedMat)});
      scene.newModelInstance(scene.newModel({node}), Transform{{0, 2, 20}});
    }
    {
      auto tGreenMat = scene.newMaterial(MaterialType::eTransparent);
      scene.material(tGreenMat).setColorFactor({Green, 0.5f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[0], tGreenMat)});
      scene.newModelInstance(scene.newModel({node}), Transform{{1, 2, 22}});
    }
    {
      auto tBlueMat = scene.newMaterial(MaterialType::eTransparent);
      scene.material(tBlueMat).setColorFactor({Blue, 0.5f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[0], tBlueMat)});
      scene.newModelInstance(scene.newModel({node}), Transform{{2, 2, 24}});
    }
    {
      auto tYellowMat = scene.newMaterial(MaterialType::eTransparent);
      scene.material(tYellowMat).setColorFactor({Yellow, 0.5f});
      auto node = scene.newNode();
      scene.node(node).addMeshes({scene.newMesh(primitives[0], tYellowMat)});
      scene.newModelInstance(scene.newModel({node}), Transform{{3, 2, 26}});
    }
   
    

    auto node2 = scene.newNode();
    scene.node(node2).addMeshes({scene.newMesh(primitives[1], tRedMat)});
    scene.newModelInstance(scene.newModel({node2}));
    
    
    
  }

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

  uint32_t animModel{};
  std::vector<uint32_t> insts;
  {
    std::string name = "DamagedHelmet";
    animModel = scene.loadModel(
      "assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf",
      MaterialType::eReflective);

    auto &model = scene.model(animModel);
    auto aabb = model.aabb();
    auto range = aabb.max - aabb.min;
    auto scale = 5 / std::max(std::max(range.x, range.y), range.z);
    auto center = aabb.center();
    auto halfRange = aabb.halfRange();
    Transform t{
      {-center * scale + glm::vec3{8, scale * range.y / 2.f, 8}}, glm::vec3{scale}};
    //  t.translation = -center;
    scene.newModelInstance(animModel, t, false);

    uint32_t num = 100;
    insts.reserve(num * num);
    float unit = -5;
    for(int a = 0; a < num; ++a) {
      for(int b = 0; b < num; ++b) {
        t.translation = -center * scale +
                        glm::vec3{-10 + unit * a, scale * range.y / 2.f, -10 + unit * b};
        insts.push_back(scene.newModelInstance(animModel, t, true));
      }
    }
  }

  {
    std::string name = "BoomBox";
    auto modelId = scene.loadModel(
      "assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf", MaterialType::eNone);

    auto &model = scene.model(modelId);
    auto aabb = model.aabb();
    auto range = aabb.max - aabb.min;
    auto scale = 5 / std::max(std::max(range.x, range.y), range.z);
    auto center = aabb.center();
    auto halfRange = aabb.halfRange();
    Transform t{
      {-center * scale + glm::vec3{8, scale * range.y / 2.f, 8}}, glm::vec3{scale}};

    uint32_t num = 100;
    float unit = 5;
    for(int a = 0; a < num; ++a) {
      for(int b = 0; b < num; ++b) {
        t.translation =
          -center * scale +
          glm::vec3{-100 + unit * a, scale * range.y / 2.f + 30, unit * b - 100};
        scene.newModelInstance(modelId, t, true);
      }
    }
  }

//  {
//    std::string name = "MetalRoughSpheres";
//    auto modelId = scene.loadModel(
//      "assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf",
//      MaterialType::eReflective);
//    auto &model = scene.model(modelId);
//    auto aabb = model.aabb();
//    auto range = aabb.max - aabb.min;
//    auto scale = 200 / std::max(std::max(range.x, range.y), range.z);
//    auto center = aabb.center();
//    auto halfRange = aabb.halfRange();
//    Transform t{
//      {-center * scale + glm::vec3{100, scale * range.y / 2.f, -200}}, glm::vec3{scale}};
//    //  t.translation = -center;
//    scene.newModelInstance(modelId, t, false);
//  }

  //  { //parallel mirror
  //    auto primitive = scene.newPrimitives(
  //      PrimitiveBuilder().rectangle({0, 1, 0}, {16, 0, 16}, {0, 16*std::sqrt(2), 0}).newPrimitive())[0];
  //    auto &reflectiveMat = scene.material(scene.newMaterial(MaterialType::eReflective));
  //    reflectiveMat.setColorFactor({White, 1.f}).setPbrFactor({0, 0.3, 0.5, 0});
  //    auto mesh = scene.newMesh(primitive, reflectiveMat.id());
  //    auto node = scene.newNode();
  //    scene.node(node).addMeshes({mesh});
  //    auto model = scene.newModel({node});
  //    scene.newModelInstance(model, Transform{{0, 0, -30}});
  //    scene.newModelInstance(model, Transform{{-30, 0, 0}});
  //  }

  std::vector<uint32_t> balls;
  uint32_t ballModel, boxModel;
  uint32_t ballMat;
  {
    auto primitives =
      scene.newPrimitives(PrimitiveBuilder()
                            .sphere({}, 1.f)
                            .newPrimitive()
                            .box({}, glm::vec3{0, 0, 1}, glm::vec3{1, 0, 0}, 1)
                            .newPrimitive());
    ballMat = scene.newMaterial(MaterialType::eNone);
    auto &mat = scene.material(ballMat);
    mat.setColorFactor({White, 0.5f});
    auto mesh = scene.newMesh(primitives[0], mat.id());
    auto node = scene.newNode();
    scene.node(node).addMeshes({mesh});
    ballModel = scene.newModel({node});

    mesh = scene.newMesh(primitives[1], mat.id());
    node = scene.newNode();
    scene.node(node).addMeshes({mesh});
    boxModel = scene.newModel({node});

    Transform t{};
    uint32_t num = 10;
    insts.reserve(num * num);
    float unit = 5;
    for(int a = 0; a < num; ++a) {
      for(int b = 0; b < num; ++b) {
        t.translation = glm::vec3{-10 + unit * a, 10, -10 + unit * b};
        balls.push_back(scene.newModelInstance(ballModel, t));
      }
    }
  }

  std::array<glm::vec3, 3> positions{
    glm::vec3{0, 0, 10}, glm::vec3{10, 0, 0}, glm::vec3{0, 10, 0}};
  std::array<glm::vec3, 3> normals = {
    glm::vec3{0.5774, 0.5774, 0.5774}, glm::vec3{0.5774, 0.5774, 0.5774},
    glm::vec3{0.5774, 0.5774, 0.5774}};
  uint32_t dynamicPrim{};
  {
    glm::vec2 uvs[] = {{0, 1}, {1, 1}, {0, 0}};
    uint32_t indices[] = {0, 1, 2};

    dynamicPrim = scene.newPrimitive(
      positions, normals, uvs, indices, {{0, 0, 0}, {1, 1, 1}},
      PrimitiveTopology::Triangles, true);

    auto tRedMat = scene.newMaterial(MaterialType::eTransparent);
    scene.material(tRedMat).setColorFactor({Red, 0.5f});
    auto node = scene.newNode();
    scene.node(node).addMeshes({scene.newMesh(dynamicPrim, tRedMat)});
    scene.newModelInstance(scene.newModel({node}));
  }

  uint32_t dynamicPrim2{};
  {

    dynamicPrim2 = scene.newPrimitives(
      PrimitiveBuilder()
        .box({}, glm::vec3{0, 0, 10}, glm::vec3{10, 0, 0}, 10)
        .newPrimitive(PrimitiveTopology::Triangles),
      true)[0];
    auto node = scene.newNode();
    scene.node(node).addMeshes({scene.newMesh(dynamicPrim2, texMat)});
    scene.newModelInstance(scene.newModel({node}));
  }

  std::mt19937_64 rng;
  // initialize the random number generator with time-dependent seed
  uint64_t timeSeed =
    std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32)};
  rng.seed(ss);
  // initialize a uniform distribution between 0 and 1
  std::uniform_real_distribution<float> unif(0, 1);

  auto &input = app.window().input();
  auto &camera = scene.camera();
  //    camera.setLocation({20.f, 0.f, 20.f});
  camera.setLocation({22.759, 8.61548, 26.1782});
  camera.setZFar(1e9);
  PanningCamera panningCamera{camera};
  bool pressed{false};
  auto &fpsMeter = app.fpsMeter();
  app.loop([&](uint32_t frameIdx, double elapsed) {
    static auto lastChange = std::chrono::high_resolution_clock::now();

    app.window().setWindowTitle(toString(
      "FPS: ", fpsMeter.fps(), " Frame Time: ", std::round(fpsMeter.frameTime()), " ms"));

    panningCamera.update(input);
    {
      auto &model = scene.model(animModel);
      for(auto &animation: model.animations())
        animation.animateAll(elapsed);
    }
    auto loc = camera.location();
    //    println(
    //      "camera loc:", glm::to_string(loc),
    //      " camera dir:", glm::to_string(camera.direction()));
    if(input.keyPressed[KeyW]) {
      pressed = true;
    } else if(pressed) {
      //      app.setWireFrame(!app.wireframe());
      pressed = false;
    }
    auto &sky = scene.atmosphere();
    sky.setSunDirection(sunDirection(elapsed / 1000));
    auto tStart = std::chrono::high_resolution_clock::now();
    for(auto insId: insts) {
      auto &ins = scene.modelInstance(insId);
      auto t = ins.transform();
      t.rotation =
        glm::angleAxis(glm::radians(float(elapsed) * 0.1f), glm::vec3{0, 1, 0}) *
        t.rotation;
      ins.setTransform(t);
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto tDelay = std::chrono::duration<double, std::milli>(now - lastChange).count();
    if(tDelay > 2000) {
      lastChange = now;
      for(auto insId: balls) {
        auto &ins = scene.modelInstance(insId);
        //      auto t = ins.transform();
        //      auto scale = unif(rng) * 2;
        //      t.scale = glm::vec3{scale};
        //      ins.setTransform(t);
        auto rnd = unif(rng);
        ins.changeModel(rnd > 0.5 ? boxModel : ballModel);
        if(unif(rng) > 0.5) { ins.setVisible(!ins.visible()); }
        ins.setCustomMaterial(unif(rng) > 0.5 ? blueMat : nullIdx);
      }
      auto &mat = scene.material(ballMat);
      mat.setColorFactor(unif(rng) > 0.5 ? glm::vec4{1, 0, 0, 1} : glm::vec4{0, 1, 0, 1});
    }

    //     update vertices
    auto rot = glm::angleAxis(glm::radians(float(elapsed) * 0.1f), glm::vec3(-1, 0, 1));
    for(int i = 0; i < positions.size(); ++i) {
      positions[i] = rot * positions[i];
      normals[i] = rot * normals[i];
    }
    scene.primitive(dynamicPrim)
      .update(frameIdx, positions, normals, {{0, 0, 0}, {1, 1, 1}});

    {
      static float totalElapsed = 0;
      totalElapsed += elapsed / 1000;
      auto p = PrimitiveBuilder();
      auto s = glm::abs(10 * glm::sin(totalElapsed));
      p.box({}, glm::vec3{0, 0, s}, glm::vec3{s, 0, 0}, s)
        .newPrimitive(PrimitiveTopology::Triangles);
      scene.primitive(dynamicPrim2).update(frameIdx, p);
    }
  });
  return 0;
}