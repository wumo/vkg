#pragma once
#include "vkg/base/vk_headers.hpp"
#include "vkg/base/device.hpp"
#include <functional>
#include <map>
#include <set>
#include <any>
#include <span>
#include <utility>

namespace vkg {

class Pass;
struct FrameGraphResource;
class FrameGraph;
struct RenderContext;
class Resources;
class PassBuilder;

using PassSetup = std::function<void(PassBuilder &)>;
using PassCompile = std::function<void(Resources &)>;
using PassExec = std::function<void(RenderContext &, Resources &)>;

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

class Pass {
  friend class PassBuilder;
  friend class FrameGraph;

public:
  virtual ~Pass() = default;
  virtual void setup(PassBuilder &builder){};
  virtual void compile(Resources &resources){};
  virtual void execute(RenderContext &ctx, Resources &resources){};

private:
  std::string name;
  uint32_t id;
  uint32_t order;
  std::vector<FrameGraphResource> inputs;
  std::vector<FrameGraphResource> outputs;
};

class LambdaPass: public Pass {
public:
  LambdaPass(PassSetup setup, PassCompile compile, PassExec execute);

  void setup(PassBuilder &builder) override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  PassSetup setup_;
  PassCompile compile_;
  PassExec execute_;
};

class PassBuilder {
  friend class FrameGraph;

public:
  explicit PassBuilder(FrameGraph &frameGraph, uint32_t id, Pass &pass);

  auto device() -> Device &;

  /**
   * Create new resources as the output of this pass. Resource's name shouldn't already exist.
   */
  auto create(std::string name, ResourceType type) -> FrameGraphResource;
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

private:
  auto build() -> void;

  FrameGraph &frameGraph;
  const uint32_t id;
  Pass &pass;
  std::vector<FrameGraphResource> inputs;
  std::set<uint32_t> uniqueInputs;
  std::vector<FrameGraphResource> outputs;
};

class Resources {
public:
  explicit Resources(Device &device,uint32_t numResources);

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

template<class T>
concept DerivedPass = std::is_base_of<Pass, T>::value;

class FrameGraph {
  friend class PassBuilder;

public:
  explicit FrameGraph(Device &device);

  auto addPass(
    std::string name, PassSetup setup = PassSetup{}, PassCompile compile = PassCompile{},
    PassExec execute = PassExec{}) -> std::span<FrameGraphResource>;
  auto addPass(std::string name, Pass &pass) -> std::span<FrameGraphResource>;
  template<DerivedPass T, typename... Args>
  auto addPass(std::string name, Args &&... args) -> std::span<FrameGraphResource> {
    allocated.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return addPass(std::move(name), *allocated.back());
  }

  auto device() -> Device &;

  auto build() -> void;

  auto onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB) -> void;

private:
  auto create(const std::string& name, uint32_t passId, ResourceType type) -> FrameGraphResource;
  auto read(FrameGraphResource &input, uint32_t passId) -> void;
  auto write(FrameGraphResource &input, uint32_t passId) -> FrameGraphResource;
  auto check(FrameGraphResource &resource) -> void;

  Device &device_;

  std::vector<Pass *> passes;
  std::vector<uint32_t> sortedPassIds;
  std::vector<std::unique_ptr<Pass>> allocated;

  std::map<std::string_view, uint32_t> resourceIds;
  std::vector<FrameGraphResources> resRevisions;

  std::unique_ptr<Resources> resources;

  bool frozen{false};
};

}
