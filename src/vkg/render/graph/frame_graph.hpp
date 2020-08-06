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

namespace vkg {

class FrameGraph;
struct RenderContext;
class Resources;
class PassBuilder;

template<typename PassInType, typename PassOutType>
using PassSetup = std::function<PassOutType(PassBuilder &builder, PassInType &inputs)>;
using PassCompile = std::function<void(Resources &resources)>;
using PassExec = std::function<void(RenderContext &ctx, Resources &resources)>;

struct FrameGraphBaseResource {
  uint32_t id{~0u};
  uint32_t revision{0};
};

template<typename T>
struct FrameGraphResource: FrameGraphBaseResource {};

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

//template<class T, typename PassInType, typename PassOutType>
//concept DerivedPass = std::is_base_of<Pass<PassInType,PassOutType>, T>::value;

template<typename T, typename PassInType, typename PassOutType>
concept DerivedPass = std::is_base_of<Pass<PassInType, PassOutType>, T>::value;

class PassBuilder {
  friend class FrameGraph;

public:
  explicit PassBuilder(FrameGraph &frameGraph, uint32_t id, BasePass &pass);

  auto device() -> Device &;

  /**
   * Create new resources as the output of this pass. Resource's name shouldn't already exist.
   */
  template<typename T>
  auto create(const std::string &name = "") -> FrameGraphResource<T> {
    auto output = frameGraph.create<T>(id, name);
    outputs_.push_back(output);
    return output;
  }
  /**
   * Read the resource as the input of this pass.
   */
  template<typename T>
  auto read(FrameGraphResource<T> &input) -> void {
    frameGraph.read(input, id);
    errorIf(
      uniqueInputs.contains(input.id), pass_.name, " already read resource ", input.id,
      " as input");
    uniqueInputs.insert(input.id);
    inputs_.push_back(input);
  }
  /**
   * Write to the resoruce as one of the output of this pass. Resource's revision will increment by 1.
   *
   * Error will be thrown when multiple passes write to the same version of resource.
   */
  template<typename T>
  auto write(FrameGraphResource<T> &input) -> FrameGraphResource<T> {
    auto output = frameGraph.write(input, id);
    errorIf(
      uniqueInputs.contains(input.id), pass_.name, " already read resource ", input.id,
      " as input");
    uniqueInputs.insert(input.id);
    inputs_.push_back(input);
    outputs_.push_back(output);
    return output;
  }

  template<typename PassInType, typename PassOutType>
  auto addPass(
    const std::string &name, Pass<PassInType, PassOutType> &pass,
    const PassInType &inputs = {}) -> PassOutType;

  template<typename T, typename PassInType, typename... Args>
  auto newPass(const std::string &name, const PassInType &inputs, Args &&... args);

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
  vk::CommandBuffer graphics;
  vk::CommandBuffer compute;
};

class FrameGraph {
  friend class PassBuilder;

public:
  explicit FrameGraph(Device &device);

  template<typename PassInType, typename PassOutType>
  auto addPass(
    std::string name, Pass<PassInType, PassOutType> &pass, const PassInType &inputs)
    -> PassOutType {
    errorIf(frozen, "frame graph already frozen");
    auto passId = uint32_t(passes.size());
    pass.name = std::move(name);
    pass.id = passId;
    passes.push_back(&pass);
    PassBuilder builder(*this, passId, pass);
    return builder.build<PassInType, PassOutType>(pass, inputs);
  };
  template<typename T, typename PassInType, typename... Args>
  auto newPass(std::string name, const PassInType &inputs, Args &&... args) {
    allocated.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return addPass(std::move(name), (T &)*allocated.back(), inputs);
  }

  auto device() -> Device &;

  auto build() -> void;

  auto onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB) -> void;

private:
  template<typename T>
  auto create(uint32_t passId, const std::string &name = "") -> FrameGraphResource<T> {
    auto id_ = uint32_t(resRevisions.size());
    resRevisions.push_back({name, id_, {{passId, {}}}});
    return {id_, 0};
  }
  template<typename T>
  auto read(FrameGraphResource<T> &input, uint32_t passId) -> void {
    check(input);
    auto &revision = resRevisions[input.id].revisions[input.revision];
    revision.readerPasses.push_back(passId);
  }
  template<typename T>
  auto write(FrameGraphResource<T> &input, uint32_t passId) -> FrameGraphResource<T> {
    check(input);
    auto &revisions = resRevisions[input.id].revisions;
    errorIf(
      input.revision != revisions.size() - 1,
      "write to resource should be the latest, latest revision:", revisions.size() - 1,
      ", this revision:", input.revision);
    revisions.push_back({passId});
    return {input.id, uint32_t(revisions.size() - 1)};
  }
  auto check(FrameGraphBaseResource &resource) -> void;

  Device &device_;

  std::vector<BasePass *> passes;
  std::vector<uint32_t> sortedPassIds;
  std::vector<std::unique_ptr<BasePass>> allocated;

  std::vector<FrameGraphResources> resRevisions;

  std::unique_ptr<Resources> resources;

  bool frozen{false};
};

template<typename PassInType, typename PassOutType>
auto PassBuilder::addPass(
  const std::string &name, Pass<PassInType, PassOutType> &pass, const PassInType &inputs)
  -> PassOutType {
  return frameGraph.addPass(nameMangling(name), pass, inputs);
}
template<typename T, typename PassInType, typename... Args>
auto PassBuilder::newPass(
  const std::string &name, const PassInType &inputs, Args &&... args) {
  return frameGraph.newPass<T, PassInType>(
    nameMangling(name), inputs, std::forward<Args>(args)...);
}

}
