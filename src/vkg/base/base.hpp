#pragma once

#include <memory>
#include "instance.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "window.hpp"
#include "vkg/util/fps_meter.hpp"
#include "call_frame_updater.hpp"
#include "resource/buffers.hpp"
#include "resource/textures.hpp"
#include "pipeline/shaders.hpp"
#include "pipeline/render_pass.hpp"
#include "pipeline/descriptors.hpp"
#include "pipeline/descriptor_updaters.hpp"
#include "pipeline/descriptor_def.hpp"
#include "pipeline/descriptor_macro.hpp"
#include "pipeline/descriptor_pool.hpp"
#include "pipeline/pipeline.hpp"
#include "pipeline/pipeline_def.hpp"
#include "pipeline/pipeline_macro.hpp"
#include "pipeline/graphics_pipeline.hpp"
#include "pipeline/compute_pipeline_maker.hpp"
#include "pipeline/pipeline_query.hpp"

namespace vkg {
typedef void (*Updater)(double, void *);

class Base {
public:
  explicit Base(WindowConfig windowConfig = {}, FeatureConfig featureConfig = {});

  void loop(const std::function<void(double elapsedMs)> &updater = [](float) {});
  void loop(Updater updater, void *data);
  void loop(CallFrameUpdater &updater);

  auto window() -> Window &;
  auto featureConfig() const -> const FeatureConfig &;

  auto device() -> Device &;
  auto swapchain() -> Swapchain &;

protected:
  auto sync(double elapsed, const std::function<void(double)> &updater) -> void;
  auto syncTimeline(double elapsed, const std::function<void(double)> &updater) -> void;
  virtual auto resize() -> void;
  virtual auto onInit() -> void{};
  virtual void onFrame(uint32_t imageIndex, float elapsed);

  auto createDebugUtils() -> void;
  auto createSyncObjects() -> void;
  auto createCommandBuffers() -> void;

  WindowConfig windowConfig_;
  FeatureConfig featureConfig_;

  std::unique_ptr<Instance> instance;
  vk::UniqueDebugUtilsMessengerEXT callback;
  std::unique_ptr<Window> window_;
  std::unique_ptr<Device> device_;
  std::unique_ptr<Swapchain> swapchain_;

  std::vector<vk::CommandBuffer> graphicsCmdBuffers, computeCmdBuffers;

  struct Semaphores {
    vk::UniqueSemaphore imageAvailable;
    vk::UniqueSemaphore computeFinished;
    vk::UniqueSemaphore readyToCompute;
    vk::UniqueSemaphore renderFinished;

    std::vector<vk::PipelineStageFlags> renderWaitStages;
    std::vector<vk::Semaphore> renderWaits;
    std::vector<vk::Semaphore> renderSignals;
    std::vector<vk::Semaphore> computeWaits;
    std::vector<vk::PipelineStageFlags> computeWaitStages;
  };

  std::vector<Semaphores> semaphores;

  struct TimelineSemaphore {
    vk::UniqueSemaphore semaphore;
    uint64_t waitValue{0};
  };

  std::vector<TimelineSemaphore> tSemaphores;

  std::vector<vk::UniqueFence> renderFences;
  uint32_t frameIndex{0};

  FPSMeter fpsMeter;
};
}
