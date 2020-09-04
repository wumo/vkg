#include <vkg/render/ranges.hpp>
#include "vkg/util/colors.hpp"
#include "vkg/c/c_panning_camera.h"
#include "vkg/c/c_renderer.h"
#include "vkg/render/model/transform.hpp"
#include "vkg/render/model/aabb.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include <array>

using namespace vkg;
void update(uint32_t frameIdx, double elapsed, void *data) {
  auto *func = (std::function<void(uint32_t frameIdx, double elapsed)> *)data;
  (*func)(frameIdx, elapsed);
}
auto main() -> int {
  CWindowConfig windowConfig{"window", 1960, 1080};

  CFeatureConfig featureConfig{false, false, 2, true};
  auto *app = NewRenderer(windowConfig, featureConfig);
  CSceneConfig sceneConfig{
    0, 0, 0, 0, 0, 1000'0000, 1000'0000, 100'0000, 1'0000, 100'0000, 100'0000, 1000, 1,
  };
  std::string name{"Scene"};
  auto *scene = RendererAddScene(app, sceneConfig, name.c_str(), name.size());

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
  auto *sky = SceneGetAtmosphere(scene);
  AtmosphereEnable(sky, true);
  AtmosphereSetSunIntensity(sky, 10);
  AtmosphereSetSunDirection(sky, (cvec3 *)(&sunDir), 0);

  auto *shadowmap = SceneGetShadowmap(scene);
  ShadowMapEnable(shadowmap, true);
  ShadowMapSetNumCascades(shadowmap, 4);
  ShadowMapSetZFar(shadowmap, 1e3);

  //  auto lightId = scene.newLight();
  //  auto &light = scene.light(lightId);
  //  light.setColor(White);
  //  glm::vec3 loc{100, 200, 200};
  //  loc = glm::angleAxis(glm::radians(-60.f), glm::vec3{0, 1, 0}) * loc;
  //  light.setLocation(loc);
  //  light.setIntensity(1);

  glm::vec4 color;
  auto yellowMat = SceneNewMaterial(scene, CMaterialNone, false);
  color = {Yellow, 1.f};
  MaterialSetColorFactor(scene, yellowMat, (cvec4 *)&color, 0);
  auto redMat = SceneNewMaterial(scene, CMaterialNone, false);
  color = {Red, 1.f};
  MaterialSetColorFactor(scene, redMat, (cvec4 *)&color, 0);
  auto greenMat = SceneNewMaterial(scene, CMaterialNone, false);
  color = {Green, 1.f};
  MaterialSetColorFactor(scene, greenMat, (cvec4 *)&color, 0);
  auto blueMat = SceneNewMaterial(scene, CMaterialNone, false);
  color = {Blue, 1.f};
  MaterialSetColorFactor(scene, blueMat, (cvec4 *)&color, 0);
  std::string path{"./assets/TextureCoordinateTemplate.png"};
  auto colorTex = SceneNewTexture(scene, path.c_str(), path.size(), true);
  auto texMat = SceneNewMaterial(scene, CMaterialBRDF, false);
  MaterialSetColorTex(scene, texMat, colorTex);
  color = {0, 0.3, 0.4, 0};
  MaterialSetPbrFactor(scene, texMat, (cvec4 *)&color, 0);

  {
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{};
    BuildAxis(builder, (cvec3 *)&center, 0, 10.f, 0.1f, 0.5f, 50);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    auto originMesh = SceneNewMesh(scene, primitives[0], yellowMat);
    auto xMesh = SceneNewMesh(scene, primitives[1], redMat);
    auto yMesh = SceneNewMesh(scene, primitives[2], greenMat);
    auto zMesh = SceneNewMesh(scene, primitives[3], blueMat);
    Transform transform;
    auto axisNode = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{originMesh, xMesh, yMesh, zMesh};
    NodeAddMeshes(scene, axisNode, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{axisNode};
    auto axisModel = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, axisModel, reinterpret_cast<ctransform *>(&transform), false);
  }

  { // texture
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{}, x{5.f, 0.f, -5.f}, y{0.f, 10.f, 0.f};
    BuildRectangle(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    auto primitive = primitives[0];
    auto mesh = SceneNewMesh(scene, primitive, texMat);
    Transform transform;
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  uint32_t parentNodeId{}, childNodeId{};
  { // node graph
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 p1{}, p2{0, 7, 0}, up{0, 1, 0};
    BuildBoxLine(builder, (cvec3 *)&p1, 0, (cvec3 *)&p2, 0, (cvec3 *)&up, 0, 0.2f, 0.2f);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    auto primitive = primitives[0];

    auto mesh = SceneNewMesh(scene, primitive, redMat);
    Transform transform{
      {0, 7, 0}, {1, 1, 1}, glm::angleAxis(glm::radians(60.f), glm::vec3{1, 0, 0})};
    childNodeId = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, childNodeId, meshes.data(), meshes.size());

    mesh = SceneNewMesh(scene, primitive, yellowMat);
    transform = Transform{
      {0, 0, 0}, {1, 1, 1}, glm::angleAxis(glm::radians(30.f), glm::vec3{1, 0, 0})};
    parentNodeId = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    meshes = {mesh};
    NodeAddMeshes(scene, parentNodeId, meshes.data(), meshes.size());
    std::vector<uint32_t> children{childNodeId};
    NodeAddChildren(scene, parentNodeId, children.data(), children.size());
    std::vector<uint32_t> nodes{parentNodeId};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  { // brdf

    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{}, x{0, 0, 1}, y{1, 0, 0};
    BuildCheckerboard(
      builder, 100, 100, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0, 4, 4);
    BuildNewPrimitive(builder, CPrimitiveTriangles);
    x = {0, 0, 4};
    y = {4, 0, 0};
    BuildLine(builder, (cvec3 *)&x, 0, (cvec3 *)&y, 0);
    BuildNewPrimitive(builder, CPrimitiveLines);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    path = "./assets/grid.png";
    auto gridTex = SceneNewTexture(scene, path.c_str(), path.size(), true);
    auto pbrMat = SceneNewMaterial(scene, CMaterialBRDF, false);
    MaterialSetColorTex(scene, pbrMat, gridTex);
    color = {0, 1, 0, 0};
    MaterialSetPbrFactor(scene, pbrMat, (cvec4 *)&color, 0);
    auto mesh = SceneNewMesh(scene, primitives[0], pbrMat);
    Transform transform{{1, 0, 1}};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    transform = Transform{};
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);

    transform = Transform{};
    mesh = SceneNewMesh(scene, primitives[1], pbrMat);
    node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    meshes = {mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    nodes = {node};
    model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  { // lines
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 x{0, 1, 4}, y{4, 1, 0};
    BuildLine(builder, (cvec3 *)&x, 0, (cvec3 *)&y, 0);
    BuildNewPrimitive(builder, CPrimitiveLines);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);
    auto primitive = primitives[0];
    auto mat = SceneNewMaterial(scene, CMaterialNone, false);
    glm::vec4 color_ = {Green, 0.5f};
    MaterialSetColorFactor(scene, mat, (cvec4 *)&color_, 0);
    auto mesh = SceneNewMesh(scene, primitive, mat);
    Transform transform{};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  { // transparent
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{4, 0, 4}, x{2, 0, -2}, y{0, 2, 0};
    BuildRectangle(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0);
    BuildNewPrimitive(builder, CPrimitiveTriangles);
    x = {4, 0, 4};
    y = {4, 4, 4};
    BuildLine(builder, (cvec3 *)&x, 0, (cvec3 *)&y, 0);
    BuildNewPrimitive(builder, CPrimitiveLines);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    auto mat = SceneNewMaterial(scene, CMaterialTransparent, false);
    glm::vec4 color_ = {Red, 0.5f};
    MaterialSetColorFactor(scene, mat, (cvec4 *)&color_, 0);
    auto mesh = SceneNewMesh(scene, primitives[0], mat);
    Transform transform{};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);

    mesh = SceneNewMesh(scene, primitives[1], mat);
    transform = {};
    node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    meshes = {mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    nodes = {node};
    model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  {
    // brdf
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{}, x{0, 0, 1}, y{1, 0, 0};
    BuildBox(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0, 1.f);
    BuildNewPrimitive(builder, CPrimitiveTriangles);
    BuildSphere(builder, (cvec3 *)&center, 0, 2.f, 3);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    auto mat = SceneNewMaterial(scene, CMaterialReflective, false);
    glm::vec4 color_ = {Green, 1.f};
    MaterialSetColorFactor(scene, mat, (cvec4 *)&color_, 0);
    color_ = {0, 0.3, 0.4, 0};
    MaterialSetPbrFactor(scene, mat, (cvec4 *)&color_, 0);

    auto mesh = SceneNewMesh(scene, primitives[0], mat);
    Transform transform{{10, 1, 0}};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    transform = {};
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);

    mat = SceneNewMaterial(scene, CMaterialRefractive, false);
    color_ = {White, 1.f};
    MaterialSetColorFactor(scene, mat, (cvec4 *)&color_, 0);
    color_ = {0, 0.3, 0.4, 0};
    MaterialSetPbrFactor(scene, mat, (cvec4 *)&color_, 0);

    mesh = SceneNewMesh(scene, primitives[1], mat);
    transform = Transform{{0, 2, 10}};
    node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    meshes = {mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    nodes = {node};
    model = SceneNewModel(scene, nodes.data(), nodes.size());
    transform = {};
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  uint32_t animModel{};
  std::vector<uint32_t> insts;
  {
    std::string name = "DamagedHelmet";
    path = "assets/glTF-models/2.0/" + name + "/glTF/" + name + ".gltf";
    animModel = SceneLoadModel(scene, path.c_str(), path.size(), CMaterialBRDF);

    AABB aabb;
    ModelGetAABB(scene, animModel, (caabb *)&aabb);
    auto range = aabb.max - aabb.min;
    auto scale = 5 / std::max(std::max(range.x, range.y), range.z);
    auto center = aabb.center();
    auto halfRange = aabb.halfRange();
    Transform t{
      {-center * scale + glm::vec3{8, scale * range.y / 2.f, 8}}, glm::vec3{scale}};
    //  t.translation = -center;
    SceneNewModelInstance(scene, animModel, reinterpret_cast<ctransform *>(&t), false);

    uint32_t num = 100;
    insts.reserve(num * num);
    float unit = -5;
    for(int a = 0; a < num; ++a) {
      for(int b = 0; b < num; ++b) {
        t.translation = -center * scale +
                        glm::vec3{-10 + unit * a, scale * range.y / 2.f, -10 + unit * b};
        insts.push_back(SceneNewModelInstance(
          scene, animModel, reinterpret_cast<ctransform *>(&t), true));
      }
    }
  }

  std::vector<uint32_t> balls;
  uint32_t ballModel, boxModel;
  uint32_t ballMat;
  {
    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{}, x{0, 0, 1}, y{1, 0, 0};
    BuildSphere(builder, (cvec3 *)&center, 0, 1.f, 3);
    BuildNewPrimitive(builder, CPrimitiveTriangles);
    BuildBox(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0, 1.f);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, false, primitives.data());

    DeletePrimitiveBuilder(builder);

    ballMat = SceneNewMaterial(scene, CMaterialNone, false);
    glm::vec4 color_ = {White, 0.5f};
    MaterialSetColorFactor(scene, ballMat, (cvec4 *)&color_, 0);
    color_ = {0, 0.3, 0.4, 0};
    MaterialSetPbrFactor(scene, ballMat, (cvec4 *)&color_, 0);

    auto mesh = SceneNewMesh(scene, primitives[0], ballMat);
    Transform transform{};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    ballModel = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, ballModel, reinterpret_cast<ctransform *>(&transform), false);

    mesh = SceneNewMesh(scene, primitives[1], ballMat);
    transform = {};
    node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    meshes = {mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    nodes = {node};
    boxModel = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, boxModel, reinterpret_cast<ctransform *>(&transform), false);

    Transform t{};
    uint32_t num = 10;
    insts.reserve(num * num);
    float unit = 5;
    for(int a = 0; a < num; ++a) {
      for(int b = 0; b < num; ++b) {
        t.translation = glm::vec3{-10 + unit * a, 10, -10 + unit * b};
        balls.push_back(SceneNewModelInstance(
          scene, ballModel, reinterpret_cast<ctransform *>(&t), false));
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
    AABB aabb;
    dynamicPrim = SceneNewPrimitive(
      scene, (cvec3 *)positions.data(), 0, positions.size(), (cvec3 *)normals.data(), 0,
      normals.size(), (cvec2 *)uvs, 0, 3, indices, 3, (caabb *)&aabb, CPrimitiveTriangles,
      true);

    auto mat = SceneNewMaterial(scene, CMaterialTransparent, false);
    glm::vec4 color_ = {Red, 0.5f};
    MaterialSetColorFactor(scene, mat, (cvec4 *)&color_, 0);

    auto mesh = SceneNewMesh(scene, dynamicPrim, mat);
    Transform transform{};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  uint32_t dynamicPrim2{};
  {

    auto *builder = NewPrimitiveBuilder();
    glm::vec3 center{}, x{0, 0, 10}, y{10, 0, 0};
    BuildBox(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0, 10.f);
    BuildNewPrimitive(builder, CPrimitiveTriangles);

    auto numPrimitives = BuilderNumPrimitives(builder);
    std::vector<uint32_t> primitives(numPrimitives);
    SceneNewPrimitives(scene, builder, true, primitives.data());

    DeletePrimitiveBuilder(builder);

    dynamicPrim2 = primitives[0];
    auto mesh = SceneNewMesh(scene, dynamicPrim2, texMat);
    Transform transform{};
    auto node = SceneNewNode(scene, reinterpret_cast<ctransform *>(&transform));
    std::vector<uint32_t> meshes{mesh};
    NodeAddMeshes(scene, node, meshes.data(), meshes.size());
    std::vector<uint32_t> nodes{node};
    auto model = SceneNewModel(scene, nodes.data(), nodes.size());
    SceneNewModelInstance(
      scene, model, reinterpret_cast<ctransform *>(&transform), false);
  }

  std::mt19937_64 rng;
  // initialize the random number generator with time-dependent seed
  uint64_t timeSeed =
    std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32)};
  rng.seed(ss);
  // initialize a uniform distribution between 0 and 1
  std::uniform_real_distribution<float> unif(0, 1);

  auto *window = RendererGetWindow(app);
  auto *input = WindowGetInput(window);
  auto *camera = SceneGetCamera(scene);
  glm::vec3 loc{22.759, 8.61548, 26.1782};
  CameraSetLocation(camera, (cvec3 *)&loc, 0);
  CameraSetZFar(camera, 1e9);
  auto *panningCamera = NewPanningCamera(camera);
  bool pressed{false};
  std::function<void(uint32_t frameIdx, double elapsed)> updater = [&](
                                                                     uint32_t frameIdx,
                                                                     double elapsed) {
    static auto lastChange = std::chrono::high_resolution_clock::now();

    PanningCameraUpdate(panningCamera, input);
    //    {
    //      auto &model = scene.model(animModel);
    //      for(auto &animation: model.animations())
    //        animation.animateAll(elapsed);
    //    }
    //    println(
    //      "camera loc:", glm::to_string(loc),
    //      " camera dir:", glm::to_string(camera.direction()));
    //    if(input.keyPressed[KeyW]) {
    //      pressed = true;
    //    } else if(pressed) {
    //      //      app.setWireFrame(!app.wireframe());
    //      pressed = false;
    //    }
    sunDir = sunDirection(elapsed / 1000);
    AtmosphereSetSunDirection(sky, (cvec3 *)(&sunDir), 0);
    auto tStart = std::chrono::high_resolution_clock::now();
    Transform t;
    for(auto insId: insts) {
      ModelInstanceGetTransform(scene, insId, reinterpret_cast<ctransform *>(&t));
      t.rotation =
        glm::angleAxis(glm::radians(float(elapsed) * 0.1f), glm::vec3{0, 1, 0}) *
        t.rotation;
      ModelInstanceSetTransform(scene, insId, reinterpret_cast<ctransform *>(&t));
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto tDelay = std::chrono::duration<double, std::milli>(now - lastChange).count();
    if(tDelay > 2000) {
      lastChange = now;
      for(auto insId: balls) {
        //      auto t = ins.transform();
        //      auto scale = unif(rng) * 2;
        //      t.scale = glm::vec3{scale};
        //      ins.setTransform(t);
        auto rnd = unif(rng);
        ModelInstanceChangeModel(scene, insId, rnd > 0.5 ? boxModel : ballModel);
        if(unif(rng) > 0.5) {
          ModelInstanceSetVisible(scene, insId, !ModelInstanceGetVisible(scene, insId));
        }
        ModelInstanceSetCustomMaterial(scene, insId, unif(rng) > 0.5 ? blueMat : nullIdx);
      }
      glm::vec4 color_ = unif(rng) > 0.5 ? glm::vec4{1, 0, 0, 1} : glm::vec4{0, 1, 0, 1};
      MaterialSetColorFactor(scene, ballMat, reinterpret_cast<cvec4 *>(&color), 0);
    }

    //     update vertices
    auto rot = glm::angleAxis(glm::radians(float(elapsed) * 0.1f), glm::vec3(-1, 0, 1));
    for(int i = 0; i < positions.size(); ++i) {
      positions[i] = rot * positions[i];
      normals[i] = rot * normals[i];
    }
    AABB aabb;
    PrimitiveUpdate(
      scene, dynamicPrim, frameIdx, (cvec3 *)positions.data(), 0, positions.size(),
      (cvec3 *)normals.data(), 0, normals.size(), (caabb *)&aabb);

    {
      static float totalElapsed = 0;
      totalElapsed += elapsed / 1000;
      auto s = glm::abs(10 * glm::sin(totalElapsed));
      auto *builder = NewPrimitiveBuilder();
      glm::vec3 center{}, x{0, 0, s}, y{s, 0, 0};
      BuildBox(builder, (cvec3 *)&center, 0, (cvec3 *)&x, 0, (cvec3 *)&y, 0, s);
      BuildNewPrimitive(builder, CPrimitiveTriangles);

      PrimitiveUpdateFromBuilder(scene, dynamicPrim2, frameIdx, builder);

      DeletePrimitiveBuilder(builder);
    }
  };
  RendererLoopFuncPtr(app, update, &updater);
  DeleteRenderer(app);
  return 0;
}