#pragma once
#include <cstdint>
#include "vkg/base/base.hpp"
#include "vkg/render/ranges.hpp"

namespace vkg {
class FrameUpdatable {
  friend class SceneSetupPass;

protected:
  virtual void updateFrame(uint32_t frameIdx, vk::CommandBuffer cb) = 0;
  /**ticket for scheduled update*/
  uint32_t ticket{nullIdx};
};
}
