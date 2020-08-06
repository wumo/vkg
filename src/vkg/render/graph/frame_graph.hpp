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

struct FrameGraphResource;
class FrameGraph;
struct RenderContext;
class Resources;
class PassBuilder;

template<typename PassInType, typename PassOutType>
using PassSetup = std::function<PassOutType(PassBuilder &builder, PassInType &inputs)>;
using PassCompile = std::function<void(Resources &resources)>;
using PassExec = std::function<void(RenderContext &ctx, Resources &resources)>;

enum class ResourceType { eBuffer, eImage, eValue };
struct FrameGraphResource {
  std::string name;
  uint32_t id;
  uint32_t revision;
  ResourceType type;
};

struct FrameGraphResourcePass {
  uint32_t writerPass;
  std::vector<uint32_t> readerPasses;
};

struct FrameGraphResources {
  std::string name;
  uint32_t id;
  ResourceType type;
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
  std::vector<FrameGraphResource> inputs_;
  std::vector<FrameGraphResource> outputs_;
};

template<typename PassInType, typename PassOutType>
class Pass: public BasePass {

public:
  virtual auto setup(PassBuilder &builder, const PassInType &inputs) -> PassOutType = 0;
};

//template<class T, typename PassInType, typename PassOutType>
//concept DerivedPass = std::is_base_of<Pass<PassInType,PassOutType>, T>::value;

class PassBuilder {
  friend class FrameGraph;

public:
  explicit PassBuilder(FrameGraph &frameGraph, uint32_t id, BasePass &pass);

  auto device() -> Device &;

  /**
   * Create new resources as the output of this pass. Resource's name shouldn't already exist.
   */
  auto create(const std::string &name, ResourceType type) -> FrameGraphResource;
  /**
   * Read the resource as the input of this pass.
   */
  auto read(FrameGraphResource &input) -> void;
  /**
   * Write to the resoruce as one of the output of this pass. Resource's revision will increment by 1.
   *
   * Error will be thrown when multiple passes write to the same version of resource.
   */
  auto write(FrameGraphResource &input) -> FrameGraphResource;

  template<typename PassInType, typename PassOutType>
  auto addLambdaPass(
    const std::string &name, PassSetup<PassInType, PassOutType> setup,
    PassCompile compile = [](auto &) {}, PassExec execute = [](auto &, auto &) {},
    const PassInType &inputs = {}) -> PassOutType;

  template<typename PassInType, typename PassOutType>
  auto addPass(
    const std::string &name, Pass<PassInType, PassOutType> &pass,
    const PassInType &inputs = {}) -> PassOutType;
  template<typename T, typename PassInType, typename PassOutType, typename... Args>
  auto newPass(const std::string &name, const PassInType &inputs, Args &&... args)
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

  FrameGraph &frameGraph;
  const uint32_t id;
  BasePass &pass_;
  std::vector<FrameGraphResource> inputs_;
  std::set<uint32_t> uniqueInputs;
  std::vector<FrameGraphResource> outputs_;
};

class Resources {
public:
  explicit Resources(Device &device, uint32_t numResources);

  template<typename T>
  auto set(FrameGraphResource &resource, T res) -> Resources & {
    physicalResources.at(resource.id) = res;
    return *this;
  }

  template<typename T>
  auto get(FrameGraphResource &resource) -> T {
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
  template<typename T, typename PassInType, typename PassOutType, typename... Args>
  auto newPass(std::string name, const PassInType &inputs, Args &&... args)
    -> PassOutType {
    allocated.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return addPass(
      std::move(name), (Pass<PassInType, PassOutType> &)*allocated.back(), inputs);
  }

  auto device() -> Device &;

  auto build() -> void;

  auto onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB) -> void;

private:
  auto create(const std::string &name, uint32_t passId, ResourceType type)
    -> FrameGraphResource;
  auto read(FrameGraphResource &input, uint32_t passId) -> void;
  auto write(FrameGraphResource &input, uint32_t passId) -> FrameGraphResource;
  auto check(FrameGraphResource &resource) -> void;

  Device &device_;

  std::vector<BasePass *> passes;
  std::vector<uint32_t> sortedPassIds;
  std::vector<std::unique_ptr<BasePass>> allocated;

  std::map<std::string, uint32_t> resourceIds;
  std::vector<FrameGraphResources> resRevisions;

  std::unique_ptr<Resources> resources;

  bool frozen{false};
};

template<typename PassInType, typename PassOutType>
auto PassBuilder::addLambdaPass(
  const std::string &name, PassSetup<PassInType, PassOutType> setup, PassCompile compile,
  PassExec execute, const PassInType &inputs) -> PassOutType {
  return frameGraph.addLambdaPass(name, setup, compile, execute, inputs);
}
template<typename PassInType, typename PassOutType>
auto PassBuilder::addPass(
  const std::string &name, Pass<PassInType, PassOutType> &pass, const PassInType &inputs)
  -> PassOutType {
  return frameGraph.addPass(name, pass, inputs);
}
template<typename T, typename PassInType, typename PassOutType, typename... Args>
auto PassBuilder::newPass(
  const std::string &name, const PassInType &inputs, Args &&... args) -> PassOutType {
  return frameGraph.newPass<T, PassInType, PassOutType>(
    pass_.name + name, inputs, std::forward<Args>(args)...);
}

}
