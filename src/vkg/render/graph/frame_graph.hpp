#pragma once
#include "vkg/base/vk_headers.hpp"
#include "vkg/base/device.hpp"
#include <functional>
#include <map>
#include <set>
#include <any>
#include <span>

namespace vkg {

struct Pass;
struct FrameGraphResource;
class FrameGraph;
struct RenderContext;
class Resources;
class PassBuilder;
class FrameGraphBuilder;

using PassSetup = std::function<void(PassBuilder &)>;
using PassCompile = std::function<void(Resources &)>;
using PassExec = std::function<void(RenderContext &, Resources &)>;

struct FrameGraphResource {
  std::string_view name;
  uint32_t id;
  uint32_t revision;
};

struct FrameGraphResourcePass {
  uint32_t writerPass;
  std::vector<uint32_t> readerPasses;
};

struct FrameGraphResources {
  std::string_view name;
  uint32_t id;
  std::vector<FrameGraphResourcePass> revisions;
};

struct Pass {
  PassSetup setup;
  PassCompile compile;
  PassExec execute;

  std::vector<FrameGraphResource> inputs;
  std::vector<FrameGraphResource> outputs;
};

class PassDef {
public:
  virtual void setup(PassBuilder &builder){};
  virtual void compile(Resources &resources){};
  virtual void execute(RenderContext &ctx, Resources &resources){};
};

class PassBuilder {
  friend class FrameGraphBuilder;

public:
  explicit PassBuilder(FrameGraphBuilder &builder, uint32_t id);

  auto device() -> Device &;

  /**
   * Create new resources as the output of this pass. Resource's name shouldn't already exist.
   */
  auto create(std::string_view name) -> FrameGraphResource;
  /**
   * Read the resource as the input of this pass.
   */
  auto read(FrameGraphResource &input) -> void;
  /**
   * Write to the resoruce as one of the output of this pass. Resource's revision will increment by 1.
   *
   * Error will be thrown when multiple passes write to the same version of resource.
   */
  auto write(FrameGraphResource &output) -> FrameGraphResource;

private:
  auto build() -> Pass;

  FrameGraphBuilder &builder;
  const uint32_t id;
  std::vector<FrameGraphResource> inputs;
  std::vector<FrameGraphResource> outputs;
};

class Resources {
public:
  explicit Resources(uint32_t numResources);
  
  template<typename T>
  auto set(FrameGraphResource &resource, T res) -> Resources & {
    physicalResources.at(resource.id) = res;
    return *this;
  }

  template<typename T>
  auto get(FrameGraphResource &resource) -> T {
    return std::any_cast<T>(physicalResources.at(resource.id));
  }

private:
  std::vector<std::any> physicalResources;
};

struct RenderContext {
  Device &device;
  vk::CommandBuffer graphics;
  vk::CommandBuffer compute;
};

class FrameGraph {

public:
  FrameGraph(Device &device, std::vector<Pass> passes, Resources resources);

  auto onFrame(vk::CommandBuffer graphicsCB, vk::CommandBuffer computeCB) -> void;

private:
  Device &device_;
  std::vector<Pass> passes;
  Resources resources;
};

class FrameGraphBuilder {
  friend class PassBuilder;

public:
  explicit FrameGraphBuilder(Device &device);

  auto addPass(
    PassSetup setup = PassSetup{}, PassCompile compile = PassCompile{},
    PassExec execute = PassExec{}) -> std::span<FrameGraphResource>;

  auto addPass(PassDef &pass) -> std::span<FrameGraphResource>;

  auto device() -> Device &;

  auto createFrameGraph() -> std::unique_ptr<FrameGraph>;

private:
  auto create(std::string_view name, uint32_t passId) -> FrameGraphResource;
  auto read(FrameGraphResource &input, uint32_t passId) -> void;
  auto write(FrameGraphResource &output, uint32_t passId) -> FrameGraphResource;
  auto check(FrameGraphResource &resource) -> void;

  Device &device_;

  std::vector<Pass> passes;

  std::map<std::string_view, uint32_t> resourceIds;
  std::vector<FrameGraphResources> resources;
};

}
