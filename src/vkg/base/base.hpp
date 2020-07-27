#pragma once

#include "instance.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "window.hpp"
#include "vkg/util/fps_meter.hpp"
#include <memory>
#include "call_frame_updater.hpp"

namespace vkg {
typedef void (*Updater)(double, void *);

class Base {
public:
  explicit Base(WindowConfig windowConfig = {}, FeatureConfig featureConfig = {});

  void loop(const std::function<void(double elapsedMs)> &updater = [](float) {});
  void loop(Updater updater, void *data);
  void loop(CallFrameUpdater &updater);

  auto window() -> Window &;
  auto featureConfig() const -> FeatureConfig;

protected:
  virtual auto resize() -> void;
  virtual void onFrame(uint32_t imageIndex, float elapsed);

  auto createDebutUtils() -> void;
  auto createSyncObjects() -> void;
  auto createCommandBuffers() -> void;

  WindowConfig windowConfig_;
  FeatureConfig featureConfig_;

  std::unique_ptr<Instance> instance;
  vk::UniqueDebugUtilsMessengerEXT callback;
  std::unique_ptr<Window> window_;
  std::unique_ptr<Device> device;
  std::unique_ptr<Swapchain> swapchain;

  std::vector<vk::CommandBuffer> graphicsCmdBuffers, computeCmdBuffers;

  struct Semaphores {
    vk::UniqueSemaphore imageAvailable;
    vk::UniqueSemaphore computeFinished;
    vk::UniqueSemaphore renderFinished;

    std::vector<vk::PipelineStageFlags> renderWaitStages;
    std::vector<vk::Semaphore> renderWaits;
  };

  std::vector<Semaphores> semaphores;

  std::vector<vk::UniqueFence> inFlightFrameFences;
  uint32_t frameIndex{0};

  FPSMeter fpsMeter;
};
}
