#pragma once
#include "vkg/base/vk_headers.hpp"
#include "vkg/base/device.hpp"
#include <functional>
#include <map>
#include <set>
#include <any>
#include <span>
#include <utility>
#include "vkg/util/syntactic_sugar.hpp"
#include "boost/pfr/precise.hpp"

namespace vkg {

class FrameGraph;
struct RenderContext;
class Resources;
class PassBuilder;

template<typename PassInType, typename PassOutType>
using PassSetup =
  std::function<PassOutType(PassBuilder &builder, const PassInType &inputs)>;
using PassCompile = std::function<void(Resources &resources)>;
using PassExec = std::function<void(RenderContext &ctx, Resources &resources)>;

struct FrameGraphBaseResource {
  uint32_t id{~0u};
  uint32_t revision{0};
};

template<typename T>
struct FrameGraphResource: FrameGraphBaseResource {

  // TODO don't know why, have to write this constructor to make the conversion between
  // different template parameter T an error
  FrameGraphResource(uint32_t id = ~0u, uint32_t revision = 0)
    : FrameGraphBaseResource{id, revision} {}
};

struct FrameGraphResourcePass {
  uint32_t writerPass;
  std::vector<uint32_t> readerPasses;
};

struct FrameGraphResources {
  std::string name;
  uint32_t id;
  std::vector<FrameGraphResourcePass> revisions;
};

class BasePass {
  friend class PassBuilder;
  friend class FrameGraph;

public:
  virtual ~BasePass() = default;
  virtual void compile(Resources &resources){};
  virtual void execute(RenderContext &ctx, Resources &resources){};

protected:
  std::string name;

private:
  uint32_t id;
  uint32_t order;
  std::vector<FrameGraphBaseResource> inputs_;
  std::vector<FrameGraphBaseResource> outputs_;
};

template<typename PassInType, typename PassOutType>
class Pass: public BasePass {

public:
  virtual auto setup(PassBuilder &builder, const PassInType &inputs) -> PassOutType = 0;
};

template<typename PassInType, typename PassOutType>
class LambdaPass: public Pass<PassInType, PassOutType> {
public:
  LambdaPass(
    PassSetup<PassInType, PassOutType> &&setup, PassCompile &&compile, PassExec &&exec)
    : setup_(std::move(setup)), compile_(std::move(compile)), exec_(std::move(exec)) {}

  auto setup(PassBuilder &builder, const PassInType &inputs) -> PassOutType override {
    return setup_(builder, inputs);
  }
  void compile(Resources &resources) override { compile_(resources); }
  void execute(RenderContext &ctx, Resources &resources) override {
    exec_(ctx, resources);
  }

private:
  PassSetup<PassInType, PassOutType> setup_;
  PassCompile compile_;
  PassExec exec_;
};

//template<class T, typename PassInType, typename PassOutType>
//concept DerivedPass = std::is_base_of<Pass<PassInType,PassOutType>, T>::value;

template<typename T, typename PassInType, typename PassOutType>
concept DerivedPass = std::is_base_of<Pass<PassInType, PassOutType>, T>::value;

template<typename T>
concept DerivedResource = std::is_base_of<FrameGraphBaseResource, T>::value;

class PassBuilder {
  friend class FrameGraph;

public:
  explicit PassBuilder(FrameGraph &frameGraph, uint32_t id, BasePass &pass);

  auto device() -> Device &;

  /**
   * Create new resources as the output of this pass. Resource's name shouldn't already exist.
   */
  template<typename T>
  auto create(const std::string &name = "") -> FrameGraphResource<T>;
  /**
   * Read the resource as the input of this pass.
   */
  template<DerivedResource T>
  auto read(T &input) -> void;
  /**
   * Read the struct resource as the input of this pass. each field of the struct should
   * be FrameGraphResource.
   */
  template<typename T>
  auto read(T &input) -> void;
  /**
   * Write to the resoruce as one of the output of this pass. Resource's revision will increment by 1.
   *
   * Error will be thrown when multiple passes write to the same version of resource.
   */
  template<typename T>
  auto write(const FrameGraphResource<T> &input) -> FrameGraphResource<T>;

  template<typename PassInType, typename PassOutType>
  auto addPass(
    const std::string &name, const PassInType &inputs,
    Pass<PassInType, PassOutType> &pass) -> PassOutType;

  template<typename T, typename PassInType, typename... Args>
  auto newPass(const std::string &name, const PassInType &inputs, Args &&... args);

  template<typename PassInType, typename PassOutType>
  auto newLambdaPass(
    const std::string &name, const PassInType &inputs,
    PassSetup<PassInType, PassOutType> &&setup, PassCompile &&compile, PassExec &&exec)
    -> PassOutType;

private:
  template<typename PassInType, typename PassOutType>
  auto build(Pass<PassInType, PassOutType> &pass, const PassInType &inputs)
    -> PassOutType {
    auto outputs = pass.setup(*this, inputs);
    pass.inputs_ = std::move(inputs_);
    pass.outputs_ = std::move(outputs_);
    return outputs;
  }

  auto nameMangling(std::string name) -> std::string;

  FrameGraph &frameGraph;
  const uint32_t id;
  BasePass &pass_;
  std::vector<FrameGraphBaseResource> inputs_;
  std::set<uint32_t> uniqueInputs;
  std::vector<FrameGraphBaseResource> outputs_;
};

class Resources {
public:
  explicit Resources(Device &device, uint32_t numResources);

  template<typename T>
  auto set(FrameGraphResource<T> &resource, T res) -> Resources & {
    physicalResources.at(resource.id) = res;
    return *this;
  }

  template<typename T>
  auto get(FrameGraphResource<T> &resource) -> T {
    return std::any_cast<T>(physicalResources.at(resource.id));
  }

  Device &device;

private:
  std::vector<std::any> physicalResources;
};

struct RenderContext {
  Device &device;
  uint32_t swapchainIndex{};
  vk::CommandBuffer graphics;
  vk::CommandBuffer compute;
};

class FrameGraph {
  friend class PassBuilder;

public:
  explicit FrameGraph(Device &device);

  template<typename PassInType, typename PassOutType>
  auto addPass(
    const std::string &name, const PassInType &inputs,
    Pass<PassInType, PassOutType> &pass) -> PassOutType {
    errorIf(frozen, "frame graph already frozen");
    auto passId = uint32_t(passes.size());
    pass.name = name;
    pass.id = passId;
    passes.push_back(&pass);
    PassBuilder builder(*this, passId, pass);
    return builder.build<PassInType, PassOutType>(pass, inputs);
  };
  template<typename T, typename PassInType, typename... Args>
  auto newPass(const std::string &name, const PassInType &inputs, Args &&... args) {
    allocated.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return addPass(name, inputs, (T &)*allocated.back());
  }

  template<typename PassInType, typename PassOutType>
  auto newLambdaPass(
    const std::string &name, const PassInType &inputs,
    PassSetup<PassInType, PassOutType> &&setup, PassCompile &&compile, PassExec &&exec)
    -> PassOutType {
    allocated.push_back(std::make_unique<LambdaPass<PassInType, PassOutType>>(
      std::move(setup), std::move(compile), std::move(exec)));
    return addPass(
      name, inputs, (LambdaPass<PassInType, PassOutType> &)*allocated.back());
  }

  auto device() -> Device &;

  auto build() -> void;

  auto onFrame(
    uint32_t imageIndex, vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB)
    -> void;

private:
  template<typename T>
  auto create(uint32_t passId, const std::string &name = "") -> FrameGraphResource<T> {
    auto id_ = uint32_t(resRevisions.size());
    resRevisions.push_back({name, id_, {{passId, {}}}});
    return {id_, 0};
  }
  auto read(const FrameGraphBaseResource &input, uint32_t passId) -> void;
  template<typename T>
  auto write(const FrameGraphResource<T> &input, uint32_t passId)
    -> FrameGraphResource<T> {
    check(input);
    auto &revisions = resRevisions[input.id].revisions;
    errorIf(
      input.revision != revisions.size() - 1,
      "write to resource should be the latest, latest revision:", revisions.size() - 1,
      ", this revision:", input.revision);
    revisions.push_back({passId});
    return {input.id, uint32_t(revisions.size() - 1)};
  }
  auto check(const FrameGraphBaseResource &resource) -> void;

  Device &device_;

  std::vector<BasePass *> passes;
  std::vector<uint32_t> sortedPassIds;
  std::vector<std::unique_ptr<BasePass>> allocated;

  std::vector<FrameGraphResources> resRevisions;

  std::unique_ptr<Resources> resources;

  bool frozen{false};
};

template<typename T>
auto PassBuilder::create(const std::string &name) -> FrameGraphResource<T> {
  auto output = frameGraph.create<T>(id, name);
  outputs_.push_back(output);
  return output;
}

template<DerivedResource T>
auto PassBuilder::read(T &input) -> void {
  frameGraph.read(input, id);
  errorIf(
    uniqueInputs.contains(input.id), "[", pass_.name, "$", pass_.id,
    "] already read resource (", frameGraph.resRevisions[input.id].name, "$", input.id,
    ":", input.revision, ")");
  uniqueInputs.insert(input.id);
  inputs_.push_back(input);
}

template<typename T>
auto PassBuilder::read(T &input) -> void {
  boost::pfr::for_each_field(input, [&](auto &res) {
    using U = std::remove_const_t<std::remove_reference_t<decltype(res)>>;
    static_assert(DerivedResource<U>);
    read(res);
  });
}

template<typename T>
auto PassBuilder::write(const FrameGraphResource<T> &input) -> FrameGraphResource<T> {
  auto output = frameGraph.write(input, id);
  errorIf(
    uniqueInputs.contains(input.id), "[", pass_.name, "$", pass_.id,
    "] already write resource (", frameGraph.resRevisions[input.id].name, "$", input.id,
    ":", input.revision, ")");
  uniqueInputs.insert(input.id);
  inputs_.push_back(input);
  outputs_.push_back(output);
  return output;
}

template<typename PassInType, typename PassOutType>
auto PassBuilder::addPass(
  const std::string &name, const PassInType &inputs, Pass<PassInType, PassOutType> &pass)
  -> PassOutType {
  return frameGraph.addPass(nameMangling(name), inputs, pass);
}
template<typename T, typename PassInType, typename... Args>
auto PassBuilder::newPass(
  const std::string &name, const PassInType &inputs, Args &&... args) {
  return frameGraph.newPass<T, PassInType>(
    nameMangling(name), inputs, std::forward<Args>(args)...);
}
template<typename PassInType, typename PassOutType>
auto PassBuilder::newLambdaPass(
  const std::string &name, const PassInType &inputs,
  PassSetup<PassInType, PassOutType> &&setup, PassCompile &&compile, PassExec &&exec)
  -> PassOutType {
  return nullptr;
}
}
