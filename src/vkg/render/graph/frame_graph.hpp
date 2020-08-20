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

template<typename PassInType>
using PassSetup = std::function<void(PassBuilder &builder)>;
using PassCompile = std::function<void(RenderContext &ctx, Resources &resources)>;
using PassExec = std::function<void(RenderContext &ctx, Resources &resources)>;
using PassCondition = std::function<bool()>;

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
  virtual void compile(RenderContext &ctx, Resources &resources){};
  virtual void execute(RenderContext &ctx, Resources &resources){};

  void enableIf(PassCondition &&passCondition) {
    passCondition_ = std::move(passCondition);
  }

protected:
  std::string name;

private:
  uint32_t id;
  uint32_t order;
  std::vector<FrameGraphBaseResource> inputs_;
  std::vector<FrameGraphBaseResource> outputs_;
  uint32_t parent{~0u};

  PassCondition passCondition_{[]() { return true; }};
};

template<typename PassInT, typename PassOutT>
class Pass: public BasePass {
  friend class PassBuilder;

public:
  using PassInType = PassInT;
  using PassOutType = PassOutT;

  virtual void setup(PassBuilder &builder) = 0;

  auto in() -> PassInType & { return passIn; }
  auto out() -> PassOutType & { return passOut; }

protected:
  PassInType passIn;
  PassOutType passOut;
};

template<typename PassInType, typename PassOutType>
class LambdaPass: public Pass<PassInType, PassOutType> {
public:
  LambdaPass(PassSetup<PassInType> &&setup, PassCompile &&compile, PassExec &&exec)
    : setup_(std::move(setup)), compile_(std::move(compile)), exec_(std::move(exec)) {}

  void setup(PassBuilder &builder) override { return setup_(builder); }
  void compile(RenderContext &ctx, Resources &resources) override {
    compile_(ctx, resources);
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    exec_(ctx, resources);
  }

private:
  PassSetup<PassInType> setup_;
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
   * Create new resources as the output of this pass. Resource's name shouldn't have already existed.
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
   * Error will be thrown while multiple passes write to the same version of a resource.
   */
  template<typename T>
  auto write(const FrameGraphResource<T> &input) -> FrameGraphResource<T>;

  template<typename PassInType, typename PassOutType>
  auto addPass(
    const std::string &name, const PassInType &inputs,
    Pass<PassInType, PassOutType> &pass) -> Pass<PassInType, PassOutType> &;

  template<typename T, typename... Args>
  auto newPass(
    const std::string &name, const typename T::PassInType &inputs, Args &&... args)
    -> T &;

  template<typename PassInType, typename PassOutType>
  auto newLambdaPass(
    const std::string &name, const PassInType &inputs, PassSetup<PassInType> &&setup,
    PassCompile &&compile, PassExec &&exec) -> LambdaPass<PassInType, PassOutType> &;

private:
  template<typename PassInType, typename PassOutType>
  void build(Pass<PassInType, PassOutType> &pass, const PassInType &inputs) {
    pass.passIn = inputs;
    pass.setup(*this);
    pass.inputs_ = std::move(inputs_);
    pass.outputs_ = std::move(outputs_);
  }

  auto scopedName(std::string name) -> std::string;

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
  uint32_t frameIndex{};
  uint32_t numFrames{};
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
    Pass<PassInType, PassOutType> &pass) -> Pass<PassInType, PassOutType> & {
    errorIf(frozen, "frame graph already frozen");
    auto passId = uint32_t(passes.size());
    pass.name = name;
    pass.id = passId;
    passes.push_back(&pass);
    PassBuilder builder(*this, passId, pass);
    builder.build<PassInType, PassOutType>(pass, inputs);
    return pass;
  };
  template<typename T, typename... Args>
  auto newPass(
    const std::string &name, const typename T::PassInType &inputs, Args &&... args)
    -> T & {
    allocated.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    auto idx = allocated.size() - 1;
    addPass(name, inputs, (T &)*allocated[idx]);
    return (T &)*allocated[idx];
  }

  template<typename PassInType, typename PassOutType>
  auto newLambdaPass(
    const std::string &name, const PassInType &inputs, PassSetup<PassInType> &&setup,
    PassCompile &&compile, PassExec &&exec) -> LambdaPass<PassInType, PassOutType> & {
    allocated.push_back(std::make_unique<LambdaPass<PassInType, PassOutType>>(
      std::move(setup), std::move(compile), std::move(exec)));
    auto idx = allocated.size() - 1;
    addPass(name, inputs, (LambdaPass<PassInType, PassOutType> &)*allocated[idx]);
    return (LambdaPass<PassInType, PassOutType> &)*allocated[idx];
  }

  auto device() -> Device &;

  auto build() -> void;

  auto onFrame(RenderContext &renderContext) -> void;

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
    revisions[input.revision].readerPasses.push_back(passId);
    revisions.push_back({passId});
    return {input.id, uint32_t(revisions.size() - 1)};
  }
  auto check(const FrameGraphBaseResource &resource) -> void;

  Device &device_;

  std::vector<BasePass *> passes;
  std::vector<uint32_t> sortedPassIds;
  std::vector<std::unique_ptr<BasePass>> allocated;
  std::vector<bool> enabled;

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
  -> Pass<PassInType, PassOutType> & {
  auto &p = frameGraph.addPass(scopedName(name), inputs, pass);
  p.parent = pass_.id;
  return p;
}
template<typename T, typename... Args>
auto PassBuilder::newPass(
  const std::string &name, const typename T::PassInType &inputs, Args &&... args) -> T & {
  auto &p = frameGraph.newPass<T>(scopedName(name), inputs, std::forward<Args>(args)...);
  p.parent = pass_.id;
  return p;
}
template<typename PassInType, typename PassOutType>
auto PassBuilder::newLambdaPass(
  const std::string &name, const PassInType &inputs, PassSetup<PassInType> &&setup,
  PassCompile &&compile, PassExec &&exec) -> LambdaPass<PassInType, PassOutType> & {
  auto &p = frameGraph.newLambdaPass<PassInType, PassOutType>(
    scopedName(name), inputs, std::move(setup), std::move(compile), std::move(exec));
  p.parent = pass_.id;
  return p;
}
}
